#include <iostream>
#include <map>
#include <stdio.h>

#define PAGE_SIZE 4
#define MEMORY_SIZE PAGE_SIZE*64 + 1

using namespace std;

struct segment_info
{
	int start_address;
	int size;
};

int find_segment(int size);
void FreeMemory(segment_info segment);
int Realloc(segment_info* segment, int new_size, char name);
void Malloc(int start_position, int size, char index_of_segment);
void PrintMenu();
char* memory;
map<char, segment_info> segments;

int main()
{
	char segment_index = 'A';
	char temp;

	int flag = 1;
	int i;
	int start_position;

	memory = (char*)malloc(sizeof(char) * MEMORY_SIZE);
	memset(memory, '0', MEMORY_SIZE);
	memory[MEMORY_SIZE - 1] = '\0';

	while (flag)
	{
		PrintMenu();

		int k;
		do {
			rewind(stdin);
			k = scanf_s("%d", &i);
		} while (k == 0 || i < 0 || i>6);

		switch (i)
		{
		case 1:
			cout << "Enter segment size in pages: ";
			do {
				rewind(stdin);
				k = scanf_s("%d", &i);
			} while (k == 0);

			start_position = find_segment(PAGE_SIZE * i);

			if (start_position != -1)
			{
				Malloc(start_position, i * PAGE_SIZE, segment_index);
				segment_info seg_info;

				seg_info.size = i;
				seg_info.start_address = start_position;

				segments[segment_index] = seg_info;
				segment_index++;
			}
			else
			{
				cout << "No available memory segment\n";
			}
			break;
		case 2:
			if (segments.size() != 0)
			{
				cout << "Choose segment: ";
				do
				{
					rewind(stdin);
					k = scanf_s("%c", &temp);
				} while (k == 0);

				cout << "Choose new segment size: ";
				do
				{
					rewind(stdin);
					k = scanf_s("%d", &i);
				} while (k == 0);

				map<char, segment_info>::iterator element = segments.find(temp);
				if (element == segments.end() || Realloc(&element->second, i, element->first) == -1)
				{
					cout << "Can't realloc memory\n";
				}
			}
			break;
		case 3:
			if (segments.size() != 0)
			{
				cout << "Choose segment: ";
				do {
					rewind(stdin);
					k = scanf_s("%c", &temp);
				} while (k == 0);

				map<char, segment_info>::iterator element = segments.find(temp);
				if (element != segments.end())
				{
					FreeMemory(element->second);
					segments.erase(element);
				}
			}
			break;
		case 4:
			flag = 0;
			break;
		case 5:
			system("cls");
			break;
		}
	}

	return 0;
}

void FreeMemory(segment_info segment)
{
	for (int i = segment.start_address; i < segment.size * PAGE_SIZE + segment.start_address; i++)
	{
		memory[i] = '0';
	}
}

int find_segment(int size)						// return -1 if can't to find free segment 
{
	int current_size = 0;
	int segment_start = -1;
	int flag = 0;

	for (int i = 0; i < MEMORY_SIZE - 1; i++)
	{
		if (current_size != size) {
			if (memory[i] == '0') {
				if (flag == 0) {
					segment_start = i;
					flag = 1;
				}
				current_size++;
			}
			else
			{
				flag = 0;
				current_size = 0;
			}
		}
		else
		{
			break;
		}
	}

	if (current_size == size)
	{
		return segment_start;
	}
	else
	{
		return -1;
	}
}

void Malloc(int start_position, int size, char index_of_segment)
{
	for (int i = start_position; i < start_position + size; i++)
	{
		memory[i] = index_of_segment;
	}
	return;
}

void PrintMenu()
{
	cout << memory << endl;
	cout << "1 - Malloc\n2 - Realloc\n3 - Free\n4 - Exit\n";
	return;
}

int Realloc(segment_info* segment, int new_size, char name)
{
	if (segment->size > new_size)
	{
		for (int j = segment->start_address + new_size * PAGE_SIZE; j < segment->start_address + segment->size * PAGE_SIZE; j++)
		{
			memory[j] = '0';
		}
		segment->size = new_size;
		return 0;
	}
	else if (segment->size < new_size)
	{
		int flag = 0;

		for (int i = segment->start_address + segment->size * PAGE_SIZE; i < segment->start_address + new_size * PAGE_SIZE; i++)
		{
			if (memory[i] != '0')
			{
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			for (int i = segment->start_address + segment->size * PAGE_SIZE; i < segment->start_address + new_size * PAGE_SIZE; i++)
			{
				memory[i] = name;
			}
			segment->size = new_size;
			return 0;
		}
		else
		{
			int start_pos = find_segment(new_size * PAGE_SIZE);
			if (start_pos == -1)
			{
				return -1;
			}
			FreeMemory(*segment);
			segment->start_address = start_pos;
			segment->size = new_size;
			Malloc(segment->start_address, segment->size * PAGE_SIZE, name);
			return 0;
		}

	}
}