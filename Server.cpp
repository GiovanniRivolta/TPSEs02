// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <WinSock.h>
#define DIM 1024 
#define dim 50
#define N_Games 10
#define PORT_NUM 23365
#pragma comment(lib, "Ws2_32.lib")
using namespace std;
struct User {
	char UserName[dim];
	bool Busy;
	char Game[dim];
};
struct GameList {
	char Game[dim];
}Games[N_Games];
SOCKET SClnt;
int num;
SOCKET SConn;
User UserList[dim];		//Sostituire con una lista
char buffer[DIM];
char message[DIM];
char Savailable[dim] = { "Disponibile," };
char Sbusy[dim] = { "Occupato," };
char UserError[dim] = { "Username non valido*" };

int receive(int n, int i)
{
	int ret = -1;
	if ((n = recv(SClnt, (char*)buffer, sizeof(buffer), 0)) <= 0)
	{
		closesocket(SClnt);
	}
	else
	{
		while ((i < n) && (message[i] != '*'))
		{
			message[i] = buffer[i];
			i++;
		}
		message[n - 1] = '\0';
		cout << message << endl;
		ret = 0;
	}
	return ret;
}

int checkUsername(char Name[])
{
	int i = 0;
	bool check = false;
	while ((UserList[i].UserName[0] != ' ') && (check == false))
	{
		if (strcmp(UserList[i].UserName, Name)==0)
		{
			check = true;
		}
		i++;
	}
	if (check)
		return -1;
	return 0;
}
int createNewUser(char UName[], int c)
{
	if (checkUsername(UName)==0)
	{
		strcpy(UserList[c].UserName, UName);
		UserList[c].Busy = false;
		return 1;
	}
	else
	{
		send(SClnt, (char*)&UserError, sizeof(UserError), 0);
	}
	return 0;
}

int createNewGame(char UName[], int c)
{
	if (checkUsername(UName) == 0)
	{
		strcpy(UserList[c].Game, UName);
		UserList[c].Busy = false;
		return 1;
	}
	else
	{
		send(SClnt, (char*)&UserError, sizeof(UserError), 0);
	}
	return 0;
}

void buildUserList(char result[])		//Da cambiare con aggiornamento invece che creazione
{
	int i = 0;
	while ((i < dim)&&(strcmp(UserList[i].UserName, " ")!=0))
	{
		if (i == 0)
			strcpy(result, UserList[i].UserName);
		else
			strcat(result, UserList[i].UserName);
		strcat(result, ",");
		if (UserList[i].Busy)
		{
			strcat(result, Sbusy);
		}
		else
		{
			strcat(result, Savailable);
		}
		strcat(result, UserList[i].Game);
		strcat(result, ";");
		i++;
	}
	strcat(result, "*");
}
bool checkReceivedGame(char ReceivedGame[])
{
	bool exists = false;
	for (int i = 0; i < N_Games; i++)			//Ottimizzare il for in un while((i<N_Games)&&(!exists))
	{
		if (strcmp(ReceivedGame, Games[i].Game) == 0)
		{
			exists = true;
		}
	}
	return exists;
}
int main()
{
	for (int k = 0; k < dim; k++)
	{
		strcpy(UserList[k].UserName, " ");
	}
	WSADATA WsaData;
	char UsersList[DIM];
	struct sockaddr_in SrvrAdd;
	struct sockaddr_in ClntAdd;
	int CAdd_Size, n = 0, i = 0, N_Users = 0;
	if (WSAStartup(0x0202, &WsaData) != 0)
	{
		cout << "Errore inizializzazione WinSock!" << endl;
		return -1;
	}
	if ((SConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		cout << "Errore apertura socket!" << endl;
		WSACleanup();
		return -1;
	}
	memset(&SrvrAdd, 0, sizeof(SrvrAdd));
	SrvrAdd.sin_family = AF_INET;
	SrvrAdd.sin_addr.s_addr = 0;
	SrvrAdd.sin_port = htons(PORT_NUM);
	if (bind(SConn, (struct sockaddr*)&SrvrAdd, sizeof(SrvrAdd)) == SOCKET_ERROR)
	{
		cout << "Errore associazione socket!" << endl;
		closesocket(SConn);
		WSACleanup();
		return -1;
	}
	if (listen(SConn, 1) == SOCKET_ERROR)
	{
		cout << "Errore impostazione socket!" << endl;
		closesocket(SConn);
		WSACleanup();
		return -1;
	}
	while (true)
	{
		CAdd_Size = sizeof(ClntAdd);
		SClnt = accept(SConn, (struct sockaddr*)&ClntAdd, &CAdd_Size);
		if (SClnt != INVALID_SOCKET)
		{
			while (TRUE)
			{
				/*if ((n = recv(SClnt, (char*)buffer, sizeof(buffer), 0)) <= 0)
				{
					closesocket(SClnt);
					break;
				}
				else
				{
					while ((i < n)&&(message[i]!='*'))
					{
						message[i] = buffer[i];
						i++;
					}
					message[n-1] = '\0';
					cout << message << endl;*/
				if (receive(n, i) == -1)
					break;
				else
				{
					createNewUser(message, N_Users);
					//N_Users++;
					buildUserList(UsersList);
					cout << UsersList << endl;
					int d = strlen(UsersList);
					cout << "Inviato: " << UsersList << " dimensione: " << d << endl;
					send(SClnt, (char*)&UsersList, sizeof(UsersList), 0);

					if (receive(n, i) == -1)
						break;
					else
					{
						createNewGame(message, N_Users);
						buildUserList(UsersList);
						cout << UsersList << endl;
						//num = strlen(UsersList);
						//cout << num << endl;
						//send(SClnt, (char*)&num, sizeof(int), 0);
						d = strlen(UsersList);
						send(SClnt, (char*)UsersList, strlen(UsersList), 0);
						cout << "Inviato: " << UsersList << " dimensione: " << d << endl;
					}

				}
			}
		}
	}
}