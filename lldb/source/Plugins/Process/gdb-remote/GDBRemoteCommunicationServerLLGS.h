//===-- GDBRemoteCommunicationServerLLGS.h ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERLLGS_H
#define LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERLLGS_H

#include <mutex>
#include <unordered_map>

#include "lldb/Core/Communication.h"
#include "lldb/Host/MainLoop.h"
#include "lldb/Host/common/NativeProcessProtocol.h"
#include "lldb/lldb-private-forward.h"

#include "GDBRemoteCommunicationServerCommon.h"

class StringExtractorGDBRemote;

namespace lldb_private {

namespace process_gdb_remote {

class ProcessGDBRemote;

class GDBRemoteCommunicationServerLLGS
    : public GDBRemoteCommunicationServerCommon,
      public NativeProcessProtocol::NativeDelegate {
public:
  // Constructors and Destructors
  GDBRemoteCommunicationServerLLGS(
      MainLoop &mainloop,
      const NativeProcessProtocol::Factory &process_factory);

  void SetLaunchInfo(const ProcessLaunchInfo &info);

  /// Launch a process with the current launch settings.
  ///
  /// This method supports running an lldb-gdbserver or similar
  /// server in a situation where the startup code has been provided
  /// with all the information for a child process to be launched.
  ///
  /// \return
  ///     An Status object indicating the success or failure of the
  ///     launch.
  Status LaunchProcess() override;

  /// Attach to a process.
  ///
  /// This method supports attaching llgs to a process accessible via the
  /// configured Platform.
  ///
  /// \return
  ///     An Status object indicating the success or failure of the
  ///     attach operation.
  Status AttachToProcess(lldb::pid_t pid);

  /// Wait to attach to a process with a given name.
  ///
  /// This method supports waiting for the next instance of a process
  /// with a given name and attaching llgs to that via the configured
  /// Platform.
  ///
  /// \return
  ///     An Status object indicating the success or failure of the
  ///     attach operation.
  Status AttachWaitProcess(llvm::StringRef process_name, bool include_existing);

  // NativeProcessProtocol::NativeDelegate overrides
  void InitializeDelegate(NativeProcessProtocol *process) override;

  void ProcessStateChanged(NativeProcessProtocol *process,
                           lldb::StateType state) override;

  void DidExec(NativeProcessProtocol *process) override;

  Status InitializeConnection(std::unique_ptr<Connection> connection);

protected:
  MainLoop &m_mainloop;
  MainLoop::ReadHandleUP m_network_handle_up;
  const NativeProcessProtocol::Factory &m_process_factory;
  lldb::tid_t m_current_tid = LLDB_INVALID_THREAD_ID;
  lldb::tid_t m_continue_tid = LLDB_INVALID_THREAD_ID;
  NativeProcessProtocol *m_current_process;
  NativeProcessProtocol *m_continue_process;
  std::recursive_mutex m_debugged_process_mutex;
  std::unique_ptr<NativeProcessProtocol> m_debugged_process_up;

  Communication m_stdio_communication;
  MainLoop::ReadHandleUP m_stdio_handle_up;

  lldb::StateType m_inferior_prev_state = lldb::StateType::eStateInvalid;
  llvm::StringMap<std::unique_ptr<llvm::MemoryBuffer>> m_xfer_buffer_map;
  std::mutex m_saved_registers_mutex;
  std::unordered_map<uint32_t, lldb::DataBufferSP> m_saved_registers_map;
  uint32_t m_next_saved_registers_id = 1;
  bool m_handshake_completed = false;

  PacketResult SendONotification(const char *buffer, uint32_t len);

  PacketResult SendWResponse(NativeProcessProtocol *process);

  PacketResult SendStopReplyPacketForThread(lldb::tid_t tid);

  PacketResult SendStopReasonForState(lldb::StateType process_state);

  PacketResult Handle_k(StringExtractorGDBRemote &packet);

  PacketResult Handle_qProcessInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qC(StringExtractorGDBRemote &packet);

  PacketResult Handle_QSetDisableASLR(StringExtractorGDBRemote &packet);

  PacketResult Handle_QSetWorkingDir(StringExtractorGDBRemote &packet);

  PacketResult Handle_qGetWorkingDir(StringExtractorGDBRemote &packet);

  PacketResult Handle_C(StringExtractorGDBRemote &packet);

  PacketResult Handle_c(StringExtractorGDBRemote &packet);

  PacketResult Handle_vCont(StringExtractorGDBRemote &packet);

  PacketResult Handle_vCont_actions(StringExtractorGDBRemote &packet);

  PacketResult Handle_stop_reason(StringExtractorGDBRemote &packet);

  PacketResult Handle_qRegisterInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qfThreadInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qsThreadInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_p(StringExtractorGDBRemote &packet);

  PacketResult Handle_P(StringExtractorGDBRemote &packet);

  PacketResult Handle_H(StringExtractorGDBRemote &packet);

  PacketResult Handle_I(StringExtractorGDBRemote &packet);

  PacketResult Handle_interrupt(StringExtractorGDBRemote &packet);

  // Handles $m and $x packets.
  PacketResult Handle_memory_read(StringExtractorGDBRemote &packet);

  PacketResult Handle_M(StringExtractorGDBRemote &packet);
  PacketResult Handle__M(StringExtractorGDBRemote &packet);
  PacketResult Handle__m(StringExtractorGDBRemote &packet);

  PacketResult
  Handle_qMemoryRegionInfoSupported(StringExtractorGDBRemote &packet);

  PacketResult Handle_qMemoryRegionInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_Z(StringExtractorGDBRemote &packet);

  PacketResult Handle_z(StringExtractorGDBRemote &packet);

  PacketResult Handle_s(StringExtractorGDBRemote &packet);

  PacketResult Handle_qXfer(StringExtractorGDBRemote &packet);

  PacketResult Handle_QSaveRegisterState(StringExtractorGDBRemote &packet);

  PacketResult Handle_jLLDBTraceSupported(StringExtractorGDBRemote &packet);

  PacketResult Handle_jLLDBTraceStart(StringExtractorGDBRemote &packet);

  PacketResult Handle_jLLDBTraceStop(StringExtractorGDBRemote &packet);

  PacketResult Handle_jLLDBTraceGetState(StringExtractorGDBRemote &packet);

  PacketResult Handle_jLLDBTraceGetBinaryData(StringExtractorGDBRemote &packet);

  PacketResult Handle_QRestoreRegisterState(StringExtractorGDBRemote &packet);

  PacketResult Handle_vAttach(StringExtractorGDBRemote &packet);

  PacketResult Handle_vAttachWait(StringExtractorGDBRemote &packet);

  PacketResult Handle_qVAttachOrWaitSupported(StringExtractorGDBRemote &packet);

  PacketResult Handle_vAttachOrWait(StringExtractorGDBRemote &packet);

  PacketResult Handle_D(StringExtractorGDBRemote &packet);

  PacketResult Handle_qThreadStopInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_jThreadsInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qWatchpointSupportInfo(StringExtractorGDBRemote &packet);

  PacketResult Handle_qFileLoadAddress(StringExtractorGDBRemote &packet);

  PacketResult Handle_QPassSignals(StringExtractorGDBRemote &packet);

  PacketResult Handle_g(StringExtractorGDBRemote &packet);

  void SetCurrentThreadID(lldb::tid_t tid);

  lldb::tid_t GetCurrentThreadID() const;

  void SetContinueThreadID(lldb::tid_t tid);

  lldb::tid_t GetContinueThreadID() const { return m_continue_tid; }

  Status SetSTDIOFileDescriptor(int fd);

  FileSpec FindModuleFile(const std::string &module_path,
                          const ArchSpec &arch) override;

  llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
  ReadXferObject(llvm::StringRef object, llvm::StringRef annex);

  static std::string XMLEncodeAttributeValue(llvm::StringRef value);

  virtual std::vector<std::string> HandleFeatures(
      const llvm::ArrayRef<llvm::StringRef> client_features) override;

private:
  llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>> BuildTargetXml();

  void HandleInferiorState_Exited(NativeProcessProtocol *process);

  void HandleInferiorState_Stopped(NativeProcessProtocol *process);

  NativeThreadProtocol *GetThreadFromSuffix(StringExtractorGDBRemote &packet);

  uint32_t GetNextSavedRegistersID();

  void MaybeCloseInferiorTerminalConnection();

  void ClearProcessSpecificData();

  void RegisterPacketHandlers();

  void DataAvailableCallback();

  void SendProcessOutput();

  void StartSTDIOForwarding();

  void StopSTDIOForwarding();

  // Read thread-id from packet.  If the thread-id is correct, returns it.
  // Otherwise, returns the error.
  //
  // If allow_all is true, then the pid/tid value of -1 ('all') will be allowed.
  // In any case, the function assumes that exactly one inferior is being
  // debugged and rejects pid values that do no match that inferior.
  llvm::Expected<lldb::tid_t> ReadTid(StringExtractorGDBRemote &packet,
                                      bool allow_all, lldb::pid_t default_pid);

  // For GDBRemoteCommunicationServerLLGS only
  GDBRemoteCommunicationServerLLGS(const GDBRemoteCommunicationServerLLGS &) =
      delete;
  const GDBRemoteCommunicationServerLLGS &
  operator=(const GDBRemoteCommunicationServerLLGS &) = delete;
};

} // namespace process_gdb_remote
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_PROCESS_GDB_REMOTE_GDBREMOTECOMMUNICATIONSERVERLLGS_H
