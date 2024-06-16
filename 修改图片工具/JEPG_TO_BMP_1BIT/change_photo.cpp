#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 65536

int main() {
	char portName[20];
	bool COM_NUM[257] = {false};
	int portCount = 0;
	
	printf("��ǰϵͳ�еĴ����豸��\n");
	
	// ����ö�� COM1 �� COM256
	for (int i = 1; i <= 256; i++) {
		snprintf(portName, sizeof(portName), "\\\\.\\COM%d", i);
		
		// ���Դ򿪴���
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
	
	printf("�ܹ��� %d �������豸��\n", portCount);
	
	if (portCount > 0) 
	{
		char buffer[BUFFER_SIZE];
		char output[BUFFER_SIZE];
		int i, j = 0;
		
		printf("���������ݣ����������밴Enter��Ctrl+z��Enter��\n");
		
		// ʹ�� scanf һ���Զ�ȡ���뵽 buffer �У�ֱ�� EOF
		if (scanf("%65535[^\x04]", buffer) == EOF) {
			perror("Error reading input");
			return EXIT_FAILURE;
		}
		
		// ���� buffer�������з��滻Ϊ�ո񣬲�ȥ�� 'DB'��'H' �� ';'
		for (i = 0; buffer[i] != '\0'; i++) {
			if (buffer[i] == '\n') {
				output[j++] = ' ';
			} else if (buffer[i] == 'D' && buffer[i + 1] == 'B') {
				i += 1; // ���� 'DB'
			} else if (buffer[i] == 'H' || buffer[i] == ';') {
				// ���� 'H' �� ';'
			} else {
				output[j++] = buffer[i];
			}
		}
		output[j] = '\0';  // ����ַ�����ֹ��
		
		for (int i = 1; i <= 256; i++)
		{
			if (COM_NUM[i] == false) continue;
			
			snprintf(portName, sizeof(portName), "\\\\.\\COM%d", i);
			// �򿪴���
			HANDLE hComm = CreateFile(portName, 
				GENERIC_READ | GENERIC_WRITE, 
				0, 
				NULL, 
				OPEN_EXISTING, 
				0, 
				NULL);
			
			if (hComm != INVALID_HANDLE_VALUE) {
				// ���ô��ڲ���
				DCB dcbSerialParams = { 0 };
				dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
				GetCommState(hComm, &dcbSerialParams);
				dcbSerialParams.BaudRate = CBR_19200;
				dcbSerialParams.ByteSize = 8;
				dcbSerialParams.StopBits = ONESTOPBIT;
				dcbSerialParams.Parity = NOPARITY;
				SetCommState(hComm, &dcbSerialParams);
				
				// ���������ʮ���������ݲ�����
				char* token = strtok(output, " ");
				while (token != NULL) {
					unsigned char byte;
					sscanf(token, "%hhx", &byte);
					
					DWORD bytesWritten;
					WriteFile(hComm, &byte, 1, &bytesWritten, NULL);
					
					if (bytesWritten == 1) {
						printf("�ѷ��� %02X �ֽ����ݵ����� %s��\n", byte, portName);
					} else {
						printf("�������ݵ����� %s ʧ�ܡ�\n", portName);
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
		printf("û�п��õĴ����豸��\n");
	}
	
	getchar();
	return 0;
}

