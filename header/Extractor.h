#pragma once
#include <sqlite.h>
#include <clientSocket.h>

using namespace std;

class Extractor
{

private:
	SQLite db;
	string hostIP;
	SOCKET ConnectSocket;
	ClientSocket client;
	vector<Player> players;
	vector<Player> history;


public:

	int initExtractor(PCSTR PORT) {
		this->ConnectSocket = client.ConnectToBackend(PORT);
		return this->ConnectSocket;
	}
	void remove_players() {
		string type = "del";
		for (Player player : players)
		{
			memset(player.type, 0, sizeof(player.type));
			memcpy(player.type, type.c_str(), type.size());
			client.sendPlayer(player);
			Sleep(5);
		}
		
		hostIP.clear();
		players.clear();
		db.clearPlayers();
	}

	void extract_partystate(MW2Packet pkg) {
		unsigned char* payload = pkg.payload;
		int magic = 50;
		int index = start_of_content(payload);
		int tempindex = index, namelength = 0;
		string hostip = pkg.srcIP;

		// if the saved hostIP doesnt match with the host ip from the new packet, we have a new host -> update player vector
		if (hostIP.size() > 0 && !hostIP._Equal(hostip))
		{
			remove_players();
			hostIP += hostip;
		}
		else if (hostIP.size() == 0)
		{
			hostIP += hostip;
		}

		string name,ip;
		while (index < pkg.payloadlen)
		{
			name.clear();
			ip.clear();
			namelength = get_namelength(index, payload);

			tempindex = index;
			name += extract_name(index, namelength, payload);

			index += namelength + 17;
			ip = extract_IP(index, payload);

			if (not_in_party(ip, players) and ip.compare("0.0.0.0") != 0)
			{
				push_player("add", name, ip);
				Sleep(5); // needed to not corrupt message length at backend
			}

			// search for next name in payload
			tempindex = index + 8; // plus ipv4 plus port(_q_q)


			bool searching = true;
			while (searching)
			{
				bool startNextName = true;

				for (int i = 0; i < 24; i++)
				{
					if (payload[tempindex + i] != 0)
					{
						startNextName = false;
						tempindex += i;
						break;
					}
				}
				if (startNextName)
				{
					index = tempindex + magic;
					searching = false;
				}
				else
					tempindex++;
			}
		}
		print_players();
	}

	void push_player(string type, string name, string ip)
	{
		Player player;
		string isHost;

		//error handling start
		if (name.size() > 31)
		{
			name.resize(31);
			name.push_back('\0');
		}
		if (ip.size() > 15)
		{
			ip.resize(15);
			ip.push_back('\0');
		}
		//error handling end

		// Is it the host?
		if (ip._Equal(hostIP))
			isHost += "1";
		else
			isHost += "0";

		memset(player.type, 0, sizeof(player.type));
		memset(player.name, 0, sizeof(player.name));
		memset(player.ip, 0, sizeof(player.ip));
		memset(player.isHost, 0, sizeof(player.isHost));

		memcpy(player.type, type.c_str(), type.size());
		memcpy(player.name, name.c_str(), name.size());
		memcpy(player.ip, ip.c_str(), ip.size());
		memcpy(player.isHost, isHost.c_str(), isHost.size());

		players.push_back(player);
		db.insertTable(player, "players");
		client.sendPlayer(player);
		
		/*fetch history db first
		if (not_in_party(player.ip, history))
		{ // only add players to history when not already in history
			history.push_back(player);
			//db.insertTable(player, "history");
			// keep things limited
			if (history.size() > 100)
				history.erase(history.begin());
		}
		*/
	}

	void print_players()
	{
		string playerip;
		for (Player player : players)
		{
			playerip = player.ip;
			std::cout << player.name << " : " << player.ip;
			if (playerip._Equal(hostIP))
				std::cout << " <-- (HOST)";
			std::cout << std::endl;
		}
		cout << "###########" << players.size() << "Players ###########\n" << endl;
	}

	int start_of_content(unsigned char* payload)
	{
		// partystate fragmented?  
		// 1. Fragment Ascii: [200 'E'] [12 '\f'] 0x[07][0c]
		// 2. Fragment Ascii: [105 'i'] [12 '\f'] 0x[69][0c]
		if (payload[20] == 'i' && payload[21] == '\f')
			return 26;

		return 134;
	}

	int get_namelength(int index, unsigned char* payload)
	{
		int namelength = 0;
		while (true)
		{
			bool end = true;

			for (int c = index; c < index + 5; c++)
			{
				if (payload[c] != 0)
				{
					end = false;
					break;
				}
			}
			if (end)
				break;
			namelength++;
			index++;
		}
		return namelength;
	}

	string extract_IP(int index, unsigned char* payload)
	{
		int ip[4], k = 0;
		std::string hostIP;

		for (int i = index; i < index + 4; i++)
		{
			char* hexa = convert_decimal_to_hexa(payload[i]);
			ip[k] = convert_hexa_to_decimal(hexa);
			hostIP += std::to_string(ip[k++]);
			if (i == index + 3)
				break;
			hostIP += '.';
		}

		return hostIP;
	}

	string extract_name(int index, int namelength, unsigned char* payload)
	{
		string name;
		for (int i = 0; i <= namelength; i++)
		{
			name += payload[index++];
		}
		return name;
	}

	// If new player is already in list, dont add
	bool not_in_party(string ip, std::vector<Player> items)
	{
		string playerip;
		for (Player player : items)
		{
			playerip.clear();
			playerip = player.ip;
			if (playerip._Equal(ip))
				return false;
		}
		return true;
	}

	int convert_hexa_to_decimal(char num[]) {
		int len = (int)strlen(num);
		int base = 1;
		int temp = 0;
		for (int i = len - 1; i >= 0; i--) {
			if (num[i] >= '0' && num[i] <= '9') {
				temp += (num[i] - 48) * base;
				base = base * 16;
			}
			else if (num[i] >= 'A' && num[i] <= 'F') {
				temp += (num[i] - 55) * base;
				base = base * 16;
			}
		}
		return temp;
	}

	//convert decimal to hexa
	char* convert_decimal_to_hexa(int num) {
		char* arr = new char[100]();
		char* hexaBytes = new char[100]();
		int i = 0, k = 0;
		while (num != 0) {
			int temp = 0;
			temp = num % 16;
			if (temp < 10) {
				arr[i] = temp + 48;
				i++;
			}
			else {
				arr[i] = temp + 55;
				i++;
			}
			num = num / 16;
		}
		for (int j = i - 1; j >= 0; j--)
			hexaBytes[k++] = arr[j];
		return hexaBytes;
	}
};

