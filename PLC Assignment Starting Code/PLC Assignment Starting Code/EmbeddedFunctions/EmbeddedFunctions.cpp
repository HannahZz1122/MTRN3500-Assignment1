#include "EmbeddedFunctions.h"


EmbeddedFunctions::EmbeddedFunctions() {
	GalilMngHndl = nullptr;
	GalilStream = nullptr;
}

EmbeddedFunctions::~EmbeddedFunctions() {
	GClose();
}

void EmbeddedFunctions::GOpen(String^ address, const int port) {
	if (address == nullptr)
	{
		throw gcnew ArgumentNullException("address");
	}
	if (port < 0 || port > 65535)
	{
		throw gcnew ArgumentOutOfRangeException("port", "Port number must be between 0 and 65535.");
	}
	if (GalilMngHndl != nullptr)
	{
		throw gcnew InvalidOperationException("Connection is already open.");
	}
	GalilMngHndl = gcnew TcpClient(address, port);
	GalilMngHndl->SendBufferSize = 64;
	GalilMngHndl->ReceiveBufferSize = 2048;
	GalilMngHndl->SendTimeout = 500;
	GalilMngHndl->ReceiveTimeout = 500;
	GalilMngHndl->NoDelay = true;
	GalilStream = GalilMngHndl->GetStream();
}

void EmbeddedFunctions::GClose() {
	if (GalilStream != nullptr) {
		GalilStream->Close();
		GalilStream = nullptr;
	}
	if (GalilMngHndl != nullptr) {
		GalilMngHndl->Close();
		GalilMngHndl = nullptr;
	}
}

String^ EmbeddedFunctions::GCommand(String^ command) {
	if (command == nullptr)
	{
		throw gcnew ArgumentNullException("command");
	}
	if (GalilStream == nullptr)
	{
		throw gcnew InvalidOperationException("Connection is not open.");
	}
	if (command->EndsWith("\r") == false)
	{
		command = command + "\r";
	}
	array<Byte>^ SendData;
	array<Byte>^ RecvData;
	RecvData = gcnew array<Byte>(2048);
	SendData = Text::Encoding::ASCII->GetBytes(command);
	GalilStream->Write(SendData, 0, SendData->Length);
	// we have recvtimeout so dont need Threading::Thread::Sleep(20)
	int BytesRead = GalilStream->Read(RecvData, 0, RecvData->Length);
	return Text::Encoding::ASCII->GetString(RecvData, 0, BytesRead);
}