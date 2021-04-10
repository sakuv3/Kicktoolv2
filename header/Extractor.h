#pragma once
#include <sqlite.h>

using namespace std;

class Extractor
{

private:
	string hostIP;
	vector<Player> players;
	vector<Player> history;
	SQLite db;

public:


	void extract_partystate(MW2Packet pkg) {
		unsigned char* payload = pkg.payload;
		int index = start_of_content(payload);
		int tempindex = index, namelength = 0;
		std::string hostip = pkg.srcIP;

		// if the saved hostIP doesnt match with the host ip from the new packet, we have a new host -> update player vector
		if (hostIP.size() > 0 && !hostIP._Equal(hostip))
		{
			players.clear();
			hostIP.clear();
			db.clearPlayers();
			hostIP += hostip;
		}
		else if (hostIP.size() == 0)
			hostIP += hostip;

		while (index < pkg.payloadlen)
		{
			namelength = get_namelength(index, payload);
			tempindex = index;
			char* name = new (char[namelength]);
			memset(name, 0, namelength + 1);
			memcpy(name, extract_name(index, namelength, payload), namelength);
			

			index += namelength + 17;
			std::string ip = extract_IP(index, payload);
			if (not_in_party(ip, players) and ip.compare("0.0.0.0") != 0)
			{
				push_player(name, namelength, ip);
			}

			// search for next name in payload
			int tempindex = index + 8; // plus ipv4 plus port(_q_q)
			int magic = 50;

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
		//print_players();
	}

	void push_player(char* name, int namelength, std::string ip)
	{
		Player player;

		// Insert Name
		player.name = new(char[namelength]);
		memset(player.name, 0, namelength+1);
		memcpy(player.name, name, namelength);

		// Insert IP
		player.ip += ip;

		// Is it the host?
		if (ip._Equal(hostIP))
			player.host = true;
		else
			player.host = false;


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
		players.push_back(player);
		db.insertTable(player, "players");
	}

	void print_players()
	{
		for (Player player : players)
		{
			std::cout << player.name << " : " << player.ip;
			if (player.host)
				std::cout << " <-- (HOST)";
			std::cout << std::endl;
		}
		std::cout << "###########" << players.size() << "Players ###########" << std::endl;
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

	std::string extract_IP(int index, unsigned char* payload)
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

	char* extract_name(int index, int namelength, unsigned char* payload)
	{
		char* name = new(char[namelength]);
		memset(name, 0, namelength);
		for (int i = 0; i <= namelength; i++)
		{
			name[i] = payload[index++];
		}
		return name;
	}

	// If new player is already in list, dont add
	bool not_in_party(std::string ip, std::vector<Player> items)
	{
		for (Player player : items)
		{
			if (player.ip._Equal(ip))
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

