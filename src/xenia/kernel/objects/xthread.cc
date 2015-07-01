/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/kernel/objects/xthread.h"

#include <gflags/gflags.h>

#include <cstring>

#include "xenia/base/clock.h"
#include "xenia/base/logging.h"
#include "xenia/base/math.h"
#include "xenia/base/mutex.h"
#include "xenia/base/threading.h"
#include "xenia/cpu/processor.h"
#include "xenia/kernel/kernel_state.h"
#include "xenia/kernel/native_list.h"
#include "xenia/kernel/objects/xevent.h"
#include "xenia/kernel/objects/xuser_module.h"
#include "xenia/profiling.h"

DEFINE_bool(ignore_thread_priorities, true,
            "Ignores game-specified thread priorities.");
DEFINE_bool(ignore_thread_affinities, true,
            "Ignores game-specified thread affinities.");

namespace xe {
namespace kernel {

using namespace xe::cpu;

uint32_t next_xthread_id = 0;
thread_local XThread* current_thread_tls = nullptr;
xe::mutex critical_region_;

XThread::XThread(KernelState* kernel_state, uint32_t stack_size,
                 uint32_t xapi_thread_startup, uint32_t start_address,
                 uint32_t start_context, uint32_t creation_flags)
    : XObject(kernel_state, kTypeThread),
      thread_id_(++next_xthread_id),
      thread_handle_(0),
      pcr_address_(0),
      thread_state_address_(0),
      thread_state_(0),
      priority_(0),
      affinity_(0),
      irql_(0) {
  creation_params_.stack_size = stack_size;
  creation_params_.xapi_thread_startup = xapi_thread_startup;
  creation_params_.start_address = start_address;
  creation_params_.start_context = start_context;

  // top 8 bits = processor ID (or 0 for default)
  // bit 0 = 1 to create suspended
  creation_params_.creation_flags = creation_flags;

  // Adjust stack size - min of 16k.
  if (creation_params_.stack_size < 16 * 1024) {
    creation_params_.stack_size = 16 * 1024;
  }

  apc_list_ = new NativeList(kernel_state->memory());

  event_ = object_ref<XEvent>(new XEvent(kernel_state));
  event_->Initialize(true, false);

  char thread_name[32];
  snprintf(thread_name, xe::countof(thread_name), "XThread%04X", handle());
  set_name(thread_name);

  // The kernel does not take a reference. We must unregister in the dtor.
  kernel_state_->RegisterThread(this);
}

XThread::~XThread() {
  // Unregister first to prevent lookups while deleting.
  kernel_state_->UnregisterThread(this);

  delete apc_list_;

  event_.reset();

  PlatformDestroy();

  if (thread_state_) {
    delete thread_state_;
  }
  kernel_state()->memory()->SystemHeapFree(scratch_address_);
  kernel_state()->memory()->SystemHeapFree(tls_address_);
  kernel_state()->memory()->SystemHeapFree(pcr_address_);

  if (thread_handle_) {
    // TODO(benvanik): platform kill
    XELOGE("Thread disposed without exiting");
  }
}

bool XThread::IsInThread(XThread* other) { return current_thread_tls == other; }

XThread* XThread::GetCurrentThread() {
  XThread* thread = current_thread_tls;
  if (!thread) {
    assert_always("Attempting to use kernel stuff from a non-kernel thread");
  }
  return thread;
}

uint32_t XThread::GetCurrentThreadHandle() {
  XThread* thread = XThread::GetCurrentThread();
  return thread->handle();
}

uint32_t XThread::GetCurrentThreadId(const uint8_t* pcr) {
  return xe::load_and_swap<uint32_t>(pcr + 0x2D8 + 0x14C);
}

uint32_t XThread::last_error() {
  uint8_t* p = memory()->TranslateVirtual(thread_state_address_);
  return xe::load_and_swap<uint32_t>(p + 0x160);
}

void XThread::set_last_error(uint32_t error_code) {
  uint8_t* p = memory()->TranslateVirtual(thread_state_address_);
  xe::store_and_swap<uint32_t>(p + 0x160, error_code);
}

void XThread::set_name(const std::string& name) {
  name_ = name;
  xe::threading::set_name(thread_handle_, name);
}

uint8_t GetFakeCpuNumber(uint8_t proc_mask) {
  if (!proc_mask) {
    return 0;  // is this reasonable?
  }
  assert_false(proc_mask & 0xC0);

  uint8_t cpu_number = 7 - xe::lzcnt(proc_mask);
  assert_true(1 << cpu_number == proc_mask);
  return cpu_number;
}

X_STATUS XThread::Create() {
  // Thread kernel object
  // This call will also setup the native pointer for us.
  auto kthread = reinterpret_cast<X_KTHREAD*>(CreateNative(sizeof(X_KTHREAD)));
  if (!kthread) {
    XELOGW("Unable to allocate thread object");
    return X_STATUS_NO_MEMORY;
  }

  auto module = kernel_state()->GetExecutableModule();

  // Allocate thread scratch.
  // This is used by interrupts/APCs/etc so we can round-trip pointers through.
  scratch_size_ = 4 * 16;
  scratch_address_ = memory()->SystemHeapAlloc(scratch_size_);

  // Allocate TLS block.
  // Games will specify a certain number of 4b slots that each thread will get.
  const uint32_t kDefaultTlsSlotCount = 32;
  uint32_t tls_slots = 0;
  uint32_t tls_extended_size = 0;
  if (module && module->xex_header()) {
    const xe_xex2_header_t* header = module->xex_header();
    tls_slots = header->tls_info.slot_count ? header->tls_info.slot_count
                                            : kDefaultTlsSlotCount;
    tls_extended_size = header->tls_info.data_size;
  } else {
    tls_slots = kDefaultTlsSlotCount;
  }

  // Allocate both the slots and the extended data.
  // HACK: we're currently not using the extra memory allocated for TLS slots
  // and instead relying on native TLS slots, so don't allocate anything for
  // the slots.
  uint32_t tls_slot_size = 0;  // tls_slots * 4;
  uint32_t tls_total_size = tls_slot_size + tls_extended_size;
  tls_address_ = memory()->SystemHeapAlloc(tls_total_size);
  if (!tls_address_) {
    XELOGW("Unable to allocate thread local storage block");
    return X_STATUS_NO_MEMORY;
  }

  // Zero all of TLS.
  memory()->Fill(tls_address_, tls_total_size, 0);
  if (tls_extended_size) {
    // If game has extended data, copy in the default values.
    const xe_xex2_header_t* header = module->xex_header();
    assert_not_zero(header->tls_info.raw_data_address);
    memory()->Copy(tls_address_, header->tls_info.raw_data_address,
                   header->tls_info.raw_data_size);
  }

  // Allocate thread state block from heap.
  // http://www.microsoft.com/msj/archive/s2ce.aspx
  // This is set as r13 for user code and some special inlined Win32 calls
  // (like GetLastError/etc) will poke it directly.
  // We try to use it as our primary store of data just to keep things all
  // consistent.
  // 0x000: pointer to tls data
  // 0x100: pointer to TEB(?)
  // 0x10C: Current CPU(?)
  // 0x150: if >0 then error states don't get set (DPC active bool?)
  // TEB:
  // 0x14C: thread id
  // 0x160: last error
  // So, at offset 0x100 we have a 4b pointer to offset 200, then have the
  // structure.
  pcr_address_ = memory()->SystemHeapAlloc(sizeof(X_KPCR));
  if (!pcr_address_) {
    XELOGW("Unable to allocate thread state block");
    return X_STATUS_NO_MEMORY;
  }

  // Allocate processor thread state.
  // This is thread safe.
  thread_state_ = new ThreadState(kernel_state()->processor(), thread_id_,
                                  ThreadStackType::kUserStack, 0,
                                  creation_params_.stack_size, pcr_address_);
  XELOGI("XThread%04X (%X) Stack: %.8X-%.8X", handle(),
         thread_state_->thread_id(), thread_state_->stack_limit(),
         thread_state_->stack_base());

  // Exports use this to get the kernel.
  thread_state_->context()->kernel_state = kernel_state_;

  uint8_t proc_mask =
      static_cast<uint8_t>(creation_params_.creation_flags >> 24);

  // Setup PCR
  auto pcr = memory()->TranslateVirtual<X_KPCR*>(pcr_address_);

  pcr->gp_save.gpr_13_save = pcr_address_;
  pcr->prcb_data.current_thread = guest_object();

  pcr->stack_base =
      thread_state_->stack_address() + thread_state_->stack_size();
  pcr->stack_limit = thread_state_->stack_address();

  pcr->prcb_data.number = GetFakeCpuNumber(proc_mask);  // Current CPU
  pcr->prcb_data.dpc_routine_active = 0;  // Prevents last error getting set

  // Setup the thread state block (last error/etc).
  kthread->header.type = 6;
  kthread->header.wait_list_flink =
      memory()->TranslateHost(&kthread->header.wait_list_flink);
  kthread->header.wait_list_blink =
      memory()->TranslateHost(&kthread->header.wait_list_flink);

  kthread->mutant_list_head.initialize(memory()->virtual_membase());

  kthread->timer_wait_block.wait_list_entry.initialize(
      memory()->virtual_membase());
  kthread->timer_wait_block.thread = guest_object();
  kthread->timer_wait_block.object = memory()->TranslateHost(&kthread->timer);
  kthread->timer_wait_block.wait_key = 0x102;
  kthread->timer_wait_block.wait_type = 1;

  kthread->stack_limit = thread_state_->stack_address();
  kthread->stack_base =
      thread_state_->stack_address() + thread_state_->stack_size();

  kthread->tls_data = tls_address_;

  kthread->state = 0;
  kthread->apc_list_head[0].initialize(memory()->virtual_membase());
  kthread->apc_list_head[1].initialize(memory()->virtual_membase());

  kthread->process_ptr = kernel_state_->process_info_block_address();
  kthread->apc_queueable = 1;

  kthread->msr.msr_enable_mask = 0xFDFFD7FF;

  kthread->stack_allocated_base =
      thread_state_->stack_address() + thread_state_->stack_size();

  kthread->thread_list_entry.initialize(memory()->virtual_membase());
  kthread->queue_list_entry.initialize(memory()->virtual_membase());
  kthread->create_time = Clock::QueryGuestSystemTime();
  kthread->active_timer_list_head.initialize(memory()->virtual_membase());

  kthread->thread_id_ptr = thread_id_;  // FIXME: Proper?
  kthread->start_address = creation_params_.start_address;
  kthread->irp_list.initialize(memory()->virtual_membase());
  kthread->create_options = creation_params_.creation_flags;

  // FIXME: Old code set vscr[1] to (int)1. What do we do?

  X_STATUS return_code = PlatformCreate();
  if (XFAILED(return_code)) {
    XELOGW("Unable to create platform thread (%.8X)", return_code);
    return return_code;
  }

  // uint32_t proc_mask = creation_params_.creation_flags >> 24;
  if (proc_mask) {
    SetAffinity(proc_mask);
  }

  return X_STATUS_SUCCESS;
}

X_STATUS XThread::Exit(int exit_code) {
  // TODO(benvanik): set exit code in thread state block

  // TODO(benvanik); dispatch events? waiters? etc?
  if (event_) {
    event_->Set(0, false);
  }
  RundownAPCs();

  kernel_state()->OnThreadExit(this);

  // NOTE: unless PlatformExit fails, expect it to never return!
  current_thread_tls = nullptr;
  xe::Profiler::ThreadExit();

  Release();
  X_STATUS return_code = PlatformExit(exit_code);
  if (XFAILED(return_code)) {
    return return_code;
  }
  return X_STATUS_SUCCESS;
}

#if XE_PLATFORM_WIN32

static uint32_t __stdcall XThreadStartCallbackWin32(void* param) {
  auto thread = reinterpret_cast<XThread*>(param);
  thread->set_name(thread->name());
  xe::Profiler::ThreadEnter(thread->name().c_str());
  current_thread_tls = thread;
  thread->Execute();
  current_thread_tls = nullptr;
  xe::Profiler::ThreadExit();
  thread->Release();
  return 0;
}

X_STATUS XThread::PlatformCreate() {
  Retain();
  bool suspended = creation_params_.creation_flags & 0x1;
  const size_t kStackSize = 16 * 1024 * 1024;  // let's do the stupid thing
  thread_handle_ = CreateThread(
      NULL, kStackSize, (LPTHREAD_START_ROUTINE)XThreadStartCallbackWin32, this,
      suspended ? CREATE_SUSPENDED : 0, NULL);
  if (!thread_handle_) {
    uint32_t last_error = GetLastError();
    // TODO(benvanik): translate?
    XELOGE("CreateThread failed with %d", last_error);
    return last_error;
  }

  if (creation_params_.creation_flags & 0x60) {
    SetThreadPriority(thread_handle_,
                      creation_params_.creation_flags & 0x20 ? 1 : 0);
  }

  return X_STATUS_SUCCESS;
}

void XThread::PlatformDestroy() {
  CloseHandle(reinterpret_cast<HANDLE>(thread_handle_));
  thread_handle_ = NULL;
}

X_STATUS XThread::PlatformExit(int exit_code) {
  // NOTE: does not return.
  ExitThread(exit_code);
  return X_STATUS_SUCCESS;
}

#else

static void* XThreadStartCallbackPthreads(void* param) {
  auto thread = object_ref<XThread>(reinterpret_cast<XThread*>(param));
  xe::Profiler::ThreadEnter(thread->name().c_str());
  current_thread_tls = thread.get();
  thread->Execute();
  current_thread_tls = nullptr;
  xe::Profiler::ThreadExit();
  return 0;
}

X_STATUS XThread::PlatformCreate() {
  pthread_attr_t attr;

  pthread_attr_init(&attr);
  // TODO(benvanik): this shouldn't be necessary
  // pthread_attr_setstacksize(&attr, creation_params_.stack_size);

  int result_code;
  if (creation_params_.creation_flags & 0x1) {
#if XE_PLATFORM_MAC
    result_code = pthread_create_suspended_np(
        reinterpret_cast<pthread_t*>(&thread_handle_), &attr,
        &XThreadStartCallbackPthreads, this);
#else
    // TODO(benvanik): pthread_create_suspended_np on linux
    assert_always();
#endif  // OSX
  } else {
    result_code = pthread_create(reinterpret_cast<pthread_t*>(&thread_handle_),
                                 &attr, &XThreadStartCallbackPthreads, this);
  }

  pthread_attr_destroy(&attr);

  switch (result_code) {
    case 0:
      // Succeeded!
      return X_STATUS_SUCCESS;
    default:
    case EAGAIN:
      return X_STATUS_NO_MEMORY;
    case EINVAL:
    case EPERM:
      return X_STATUS_INVALID_PARAMETER;
  }
}

void XThread::PlatformDestroy() {
  // No-op?
}

X_STATUS XThread::PlatformExit(int exit_code) {
  // NOTE: does not return.
  pthread_exit(reinterpret_cast<void*>(exit_code));
  return X_STATUS_SUCCESS;
}

#endif  // WIN32

void XThread::Execute() {
  XELOGKERNEL("XThread::Execute thid %d (handle=%.8X, '%s', native=%.8X)",
              thread_id_, handle(), name_.c_str(),
              xe::threading::current_thread_id());

  // Let the kernel know we are starting.
  kernel_state()->OnThreadExecute(this);

  // All threads get a mandatory sleep. This is to deal with some buggy
  // games that are assuming the 360 is so slow to create threads that they
  // have time to initialize shared structures AFTER CreateThread (RR).
  xe::threading::Sleep(std::chrono::milliseconds::duration(100));

  int exit_code = 0;

  // If a XapiThreadStartup value is present, we use that as a trampoline.
  // Otherwise, we are a raw thread.
  if (creation_params_.xapi_thread_startup) {
    uint64_t args[] = {creation_params_.start_address,
                       creation_params_.start_context};
    kernel_state()->processor()->Execute(thread_state_,
                                         creation_params_.xapi_thread_startup,
                                         args, xe::countof(args));
  } else {
    // Run user code.
    uint64_t args[] = {creation_params_.start_context};
    exit_code = (int)kernel_state()->processor()->Execute(
        thread_state_, creation_params_.start_address, args, xe::countof(args));
    // If we got here it means the execute completed without an exit being
    // called.
    // Treat the return code as an implicit exit code.
  }

  Exit(exit_code);
}

void XThread::EnterCriticalRegion() {
  // Global critical region. This isn't right, but is easy.
  critical_region_.lock();
}

void XThread::LeaveCriticalRegion() { critical_region_.unlock(); }

uint32_t XThread::RaiseIrql(uint32_t new_irql) {
  return irql_.exchange(new_irql);
}

void XThread::LowerIrql(uint32_t new_irql) { irql_ = new_irql; }

void XThread::CheckApcs() { DeliverAPCs(this); }

void XThread::LockApc() { apc_lock_.lock(); }

void XThread::UnlockApc(bool queue_delivery) {
  bool needs_apc = apc_list_->HasPending();
  apc_lock_.unlock();
  if (needs_apc && queue_delivery) {
    QueueUserAPC(reinterpret_cast<PAPCFUNC>(DeliverAPCs), thread_handle_,
                 reinterpret_cast<ULONG_PTR>(this));
  }
}

void XThread::EnqueueApc(uint32_t normal_routine, uint32_t normal_context,
                         uint32_t arg1, uint32_t arg2) {
  LockApc();

  // Allocate APC.
  // We'll tag it as special and free it when dispatched.
  uint32_t apc_ptr = memory()->SystemHeapAlloc(XAPC::kSize);
  auto apc = reinterpret_cast<XAPC*>(memory()->TranslateVirtual(apc_ptr));

  apc->Initialize();
  apc->kernel_routine = XAPC::kDummyKernelRoutine;
  apc->rundown_routine = XAPC::kDummyRundownRoutine;
  apc->normal_routine = normal_routine;
  apc->normal_context = normal_context;
  apc->arg1 = arg1;
  apc->arg2 = arg2;
  apc->enqueued = 1;

  uint32_t list_entry_ptr = apc_ptr + 8;
  apc_list_->Insert(list_entry_ptr);

  UnlockApc(true);
}

void XThread::DeliverAPCs(void* data) {
  XThread* thread = reinterpret_cast<XThread*>(data);
  assert_true(XThread::GetCurrentThread() == thread);

  // http://www.drdobbs.com/inside-nts-asynchronous-procedure-call/184416590?pgno=1
  // http://www.drdobbs.com/inside-nts-asynchronous-procedure-call/184416590?pgno=7
  auto memory = thread->memory();
  auto processor = thread->kernel_state()->processor();
  auto apc_list = thread->apc_list();
  thread->LockApc();
  while (apc_list->HasPending()) {
    // Get APC entry (offset for LIST_ENTRY offset) and cache what we need.
    // Calling the routine may delete the memory/overwrite it.
    uint32_t apc_ptr = apc_list->Shift() - 8;
    auto apc = reinterpret_cast<XAPC*>(memory->TranslateVirtual(apc_ptr));
    bool needs_freeing = apc->kernel_routine == XAPC::kDummyKernelRoutine;

    XELOGD("Delivering APC to %.8X", uint32_t(apc->normal_routine));

    // Mark as uninserted so that it can be reinserted again by the routine.
    apc->enqueued = 0;

    // Call kernel routine.
    // The routine can modify all of its arguments before passing it on.
    // Since we need to give guest accessible pointers over, we copy things
    // into and out of scratch.
    uint8_t* scratch_ptr = memory->TranslateVirtual(thread->scratch_address_);
    xe::store_and_swap<uint32_t>(scratch_ptr + 0, apc->normal_routine);
    xe::store_and_swap<uint32_t>(scratch_ptr + 4, apc->normal_context);
    xe::store_and_swap<uint32_t>(scratch_ptr + 8, apc->arg1);
    xe::store_and_swap<uint32_t>(scratch_ptr + 12, apc->arg2);
    if (apc->kernel_routine != XAPC::kDummyKernelRoutine) {
      // kernel_routine(apc_address, &normal_routine, &normal_context,
      // &system_arg1, &system_arg2)
      uint64_t kernel_args[] = {
          apc_ptr,
          thread->scratch_address_ + 0,
          thread->scratch_address_ + 4,
          thread->scratch_address_ + 8,
          thread->scratch_address_ + 12,
      };
      processor->Execute(thread->thread_state(), apc->kernel_routine,
                         kernel_args, xe::countof(kernel_args));
    }
    uint32_t normal_routine = xe::load_and_swap<uint32_t>(scratch_ptr + 0);
    uint32_t normal_context = xe::load_and_swap<uint32_t>(scratch_ptr + 4);
    uint32_t arg1 = xe::load_and_swap<uint32_t>(scratch_ptr + 8);
    uint32_t arg2 = xe::load_and_swap<uint32_t>(scratch_ptr + 12);

    // Call the normal routine. Note that it may have been killed by the kernel
    // routine.
    if (normal_routine) {
      thread->UnlockApc(false);
      // normal_routine(normal_context, system_arg1, system_arg2)
      uint64_t normal_args[] = {normal_context, arg1, arg2};
      processor->Execute(thread->thread_state(), normal_routine, normal_args,
                         xe::countof(normal_args));
      thread->LockApc();
    }

    XELOGD("Completed delivery of APC to %.8X", uint32_t(apc->normal_routine));

    // If special, free it.
    if (needs_freeing) {
      memory->SystemHeapFree(apc_ptr);
    }
  }
  thread->UnlockApc(true);
}

void XThread::RundownAPCs() {
  assert_true(XThread::GetCurrentThread() == this);
  LockApc();
  while (apc_list_->HasPending()) {
    // Get APC entry (offset for LIST_ENTRY offset) and cache what we need.
    // Calling the routine may delete the memory/overwrite it.
    uint32_t apc_ptr = apc_list_->Shift() - 8;
    auto apc = reinterpret_cast<XAPC*>(memory()->TranslateVirtual(apc_ptr));
    bool needs_freeing = apc->kernel_routine == XAPC::kDummyKernelRoutine;

    // Mark as uninserted so that it can be reinserted again by the routine.
    apc->enqueued = 0;

    // Call the rundown routine.
    if (apc->rundown_routine == XAPC::kDummyRundownRoutine) {
      // No-op.
    } else if (apc->rundown_routine) {
      // rundown_routine(apc)
      uint64_t args[] = {apc_ptr};
      kernel_state()->processor()->Execute(thread_state(), apc->rundown_routine,
                                           args, xe::countof(args));
    }

    // If special, free it.
    if (needs_freeing) {
      memory()->SystemHeapFree(apc_ptr);
    }
  }
  UnlockApc(true);
}

int32_t XThread::QueryPriority() { return GetThreadPriority(thread_handle_); }

void XThread::SetPriority(int32_t increment) {
  priority_ = increment;
  int target_priority = 0;
  if (increment > 0x22) {
    target_priority = THREAD_PRIORITY_HIGHEST;
  } else if (increment > 0x11) {
    target_priority = THREAD_PRIORITY_ABOVE_NORMAL;
  } else if (increment < -0x22) {
    target_priority = THREAD_PRIORITY_IDLE;
  } else if (increment < -0x11) {
    target_priority = THREAD_PRIORITY_LOWEST;
  } else {
    target_priority = THREAD_PRIORITY_NORMAL;
  }
  if (!FLAGS_ignore_thread_priorities) {
    SetThreadPriority(thread_handle_, target_priority);
  }
}

void XThread::SetAffinity(uint32_t affinity) {
  // Affinity mask, as in SetThreadAffinityMask.
  // Xbox thread IDs:
  // 0 - core 0, thread 0 - user
  // 1 - core 0, thread 1 - user
  // 2 - core 1, thread 0 - sometimes xcontent
  // 3 - core 1, thread 1 - user
  // 4 - core 2, thread 0 - xaudio
  // 5 - core 2, thread 1 - user
  // TODO(benvanik): implement better thread distribution.
  // NOTE: these are logical processors, not physical processors or cores.
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  if (system_info.dwNumberOfProcessors < 6) {
    XELOGW("Too few processors - scheduling will be wonky");
  }
  SetActiveCpu(GetFakeCpuNumber(affinity));
  affinity_ = affinity;
  if (!FLAGS_ignore_thread_affinities) {
    SetThreadAffinityMask(reinterpret_cast<HANDLE>(thread_handle_), affinity);
  }
}

uint32_t XThread::active_cpu() const {
  uint8_t* pcr = memory()->TranslateVirtual(pcr_address_);
  return xe::load_and_swap<uint8_t>(pcr + 0x10C);
}

void XThread::SetActiveCpu(uint32_t cpu_index) {
  uint8_t* pcr = memory()->TranslateVirtual(pcr_address_);
  xe::store_and_swap<uint8_t>(pcr + 0x10C, cpu_index);
}

X_STATUS XThread::Resume(uint32_t* out_suspend_count) {
  DWORD result = ResumeThread(thread_handle_);
  if (result >= 0) {
    if (out_suspend_count) {
      *out_suspend_count = result;
    }
    return X_STATUS_SUCCESS;
  } else {
    return X_STATUS_UNSUCCESSFUL;
  }
}

X_STATUS XThread::Suspend(uint32_t* out_suspend_count) {
  DWORD result = SuspendThread(thread_handle_);
  if (result >= 0) {
    if (out_suspend_count) {
      *out_suspend_count = result;
    }
    return X_STATUS_SUCCESS;
  } else {
    return X_STATUS_UNSUCCESSFUL;
  }
}

X_STATUS XThread::Delay(uint32_t processor_mode, uint32_t alertable,
                        uint64_t interval) {
  int64_t timeout_ticks = interval;
  DWORD timeout_ms;
  if (timeout_ticks > 0) {
    // Absolute time, based on January 1, 1601.
    // TODO(benvanik): convert time to relative time.
    assert_always();
    timeout_ms = 0;
  } else if (timeout_ticks < 0) {
    // Relative time.
    timeout_ms = (DWORD)(-timeout_ticks / 10000);  // Ticks -> MS
  } else {
    timeout_ms = 0;
  }
  timeout_ms = Clock::ScaleGuestDurationMillis(timeout_ms);
  DWORD result = SleepEx(timeout_ms, alertable ? TRUE : FALSE);
  switch (result) {
    case 0:
      return X_STATUS_SUCCESS;
    case WAIT_IO_COMPLETION:
      return X_STATUS_USER_APC;
    default:
      return X_STATUS_ALERTED;
  }
}

void* XThread::GetWaitHandle() { return event_->GetWaitHandle(); }

XHostThread::XHostThread(KernelState* kernel_state, uint32_t stack_size,
                         uint32_t creation_flags, std::function<int()> host_fn)
    : XThread(kernel_state, stack_size, 0, 0, 0, creation_flags),
      host_fn_(host_fn) {}

void XHostThread::Execute() {
  XELOGKERNEL(
      "XThread::Execute thid %d (handle=%.8X, '%s', native=%.8X, <host>)",
      thread_id_, handle(), name_.c_str(), xe::threading::current_thread_id());

  // Let the kernel know we are starting.
  kernel_state()->OnThreadExecute(this);

  int ret = host_fn_();

  // Exit.
  Exit(ret);
}

}  // namespace kernel
}  // namespace xe
