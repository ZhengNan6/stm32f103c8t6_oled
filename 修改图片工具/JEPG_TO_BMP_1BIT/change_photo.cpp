#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 65536

int main() {
	char portName[20];
	bool COM_NUM[257] = {false};
	int portCount = 0;
	
	printf("当前系统中的串口设备：\n");
	
	// 尝试枚举 COM1 到 COM256
	for (int i = 1; i <= 256; i++) {
		snprintf(portName, sizeof(portName), "\\\\.\\COM%d", i);
		
		// 尝试打开串口
		HANDLE hComm = CreateFile(portName, 
			GENERIC_READ | GENERIC_WRITE, 
			0, 
			NULL, 
			OPEN_EXISTING, 
			0, 
			NULL);
		
		if (hComm != INVALID_HANDLE_VALUE) {
			printf("%d. %s\n", portCount + 1, portName);
			portCount++;
			CloseHandle(hComm);
			COM_NUM[i] = true;
		} else {
			//printf("Failed to open %s, Error: %ld\n", portName, GetLastError());
		}
	}
	
	printf("总共有 %d 个串口设备。\n", portCount);
	
	if (portCount > 0) 
	{
		char buffer[BUFFER_SIZE];
		char output[BUFFER_SIZE];
		int i, j = 0;
		
		printf("请输入内容，结束输入请按Enter后Ctrl+z后Enter：\n");
		
		// 使用 scanf 一次性读取输入到 buffer 中，直到 EOF
		if (scanf("%65535[^\x04]", buffer) == EOF) {
			perror("Error reading input");
			return EXIT_FAILURE;
		}
		
		// 遍历 buffer，将换行符替换为空格，并去除 'DB'、'H' 和 ';'
		for (i = 0; buffer[i] != '\0'; i++) {
			if (buffer[i] == '\n') {
				output[j++] = ' ';
			} else if (buffer[i] == 'D' && buffer[i + 1] == 'B') {
				i += 1; // 跳过 'DB'
			} else if (buffer[i] == 'H' || buffer[i] == ';') {
				// 跳过 'H' 和 ';'
			} else {
				output[j++] = buffer[i];
			}
		}
		output[j] = '\0';  // 添加字符串终止符
		
		for (int i = 1; i <= 256; i++)
		{
			if (COM_NUM[i] == false) continue;
			
			snprintf(portName, sizeof(portName), "\\\\.\\COM%d", i);
			// 打开串口
			HANDLE hComm = CreateFile(portName, 
				GENERIC_READ | GENERIC_WRITE, 
				0, 
				NULL, 
				OPEN_EXISTING, 
				0, 
				NULL);
			
			if (hComm != INVALID_HANDLE_VALUE) {
				// 配置串口参数
				DCB dcbSerialParams = { 0 };
				dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
				GetCommState(hComm, &dcbSerialParams);
				dcbSerialParams.BaudRate = CBR_19200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;
				SetCommState(hComm, &dcbSerialParams);
				
				// 解析输入的十六进制数据并发送
				char* token = strtok(output, " ");
				while (token != NULL) {
					unsigned char byte;
					sscanf(token, "%hhx", &byte);
					
					DWORD bytesWritten;
					WriteFile(hComm, &byte, 1, &bytesWritten, NULL);
					
					if (bytesWritten == 1) {
						printf("已发送 %02X 字节数据到串口 %s。\n", byte, portName);
					} else {
						printf("发送数据到串口 %s 失败。\n", portName);
					}
					
					token = strtok(NULL, " ");
				}
				
				CloseHandle(hComm);
			} else {
				printf("Failed to open %s, Error: %ld\n", portName, GetLastError());
			}
		}
	} 
	else 
	{
		printf("没有可用的串口设备。\n");
	}
	
	getchar();
	return 0;
}

