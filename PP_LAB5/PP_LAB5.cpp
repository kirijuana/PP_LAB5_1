#include "stdafx.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <vector>
using namespace std;

int front_1 = 0; //индекс для чтения из буфера
int rear_1 = 0; //индекс для записи в буфер
int front_2 = 0; //индекс для чтения из буфера
int rear_2 = 0; //индекс для записи в буфер
sem_t empty_1; //семафор, отображающий насколько буфер пуст
sem_t full_1; //семафор, отображающий насколько полон буфер
sem_t empty_2; //семафор, отображающий насколько буфер пуст
sem_t full_2; //семафор, отображающий насколько полон буфер
pthread_mutex_t mutexD; //мьютекс для операции записи
pthread_mutex_t mutexF; //мьютекс для операции чтения
int k = 100; // расстояние м/у городами
map<int, vector<int>> cannon_1;
map<int, vector<int>> cannon_2;

int z_1 = 0; // кол-во пушек
int z_2 = 0;

						   // 0 - здоровье пушки // 1 - дальность стрельбы // 2 - точность стрельбы // 3 - урон по области // 4 расположение от начала города

void cannons_generation()
{
	srand(time(NULL));
	z_1 = rand() % 10 + 1; // количество пушек
	for (int i = 0; i < z_1; i++)
	{
		cannon_1[i].push_back(10); // здоровье каждой пушки 10
		for (int j = 1; j < 5; j++)
		{
			if (j == 2)
				cannon_1[i].push_back(rand() % 4); // точность от 0 до 3
			else
				cannon_1[i].push_back(rand() % 10 + 1);
		}
	}
	
	
	z_2 = rand() % 10 + 1; // количество пушек
	for (int i = 0; i < z_2; i++)
	{
		cannon_2[i].push_back(10); // здоровье каждой пушки 10
		for (int j = 1; j < 5; j++)
		{
			if (j == 2)
				cannon_2[i].push_back(rand() % 4); // точность от 0 до 3
			else
				cannon_2[i].push_back(rand() % 10 + 1);
		}
	}
}

bool check_damage = true;
void *Cannons_1(void *param)
{
	int data, i;
	while (1)
	{
		//поместить элемент в буфер
		pthread_mutex_lock(&mutexD); //защита операции записи
		
		if (!check_damage)
			front_1 = (front_1 + 1) % z_1;

		check_damage = false;
		while (!check_damage)
		{
			if (cannon_1[front_1][0] >= 0) // если пушка жива, то стреляет
			{
				for (int j = 0; j < z_1; j++)
				{
					if ((cannon_1[front_1][1] + k >= cannon_2[j][4] + k) && (cannon_2[j][0] > 0))
					{
						cannon_2[j][0] = cannon_2[j][0] - cannon_1[front_1][3] * cannon_1[front_1][2];
						check_damage = true;
						if (cannon_2[j][0] < 0)
							printf("Город #2: пушка № %d взорвана.\n", j);
						sem_wait(&empty_2); //количество целых пушек уменьшить на 1
						sem_post(&full_2); //количество взорваных пушек увеличилось на 1
					}
				}
			}
			else
			{
				front_1++;
				if (front_1 == z_1)
				{
					printf("Целых пушек у первого города не осталось");
					break;
				}

			}
		}
		pthread_mutex_unlock(&mutexD);
	}
}
bool check_damage_2 = true;
void *Cannons_2(void *param)
{
	int result;
	while (1)
	{
		//извлечь элемент из буфера
		pthread_mutex_lock(&mutexF); //защита операции чтения
		if (!check_damage_2)
			front_2 = (front_2 + 1) % z_2;
		check_damage_2 = false;
			
		if (cannon_2[front_2][0] >= 0) // если пушка жива, то стреляет
			{
				for (int j = 0; j < z_1; j++)
				{
					if ((cannon_2[front_2][1] + k >= cannon_1[j][4] + k) && (cannon_1[j][0] > 0))
					{
						cannon_1[j][0] = cannon_1[j][0] - cannon_2[front_2][3] * cannon_2[front_2][2];
						check_damage_2 = true;
						if (cannon_1[j][0] < 0)
							printf("Город #1: пушка № %d взорвана.\n", j);
							sem_wait(&empty_1); //количество целых пушек уменьшить на 1
							sem_wait(&full_1); //количество взорванных пушек увеличить на 1
					}
				}
				break;
			}		
		pthread_mutex_unlock(&mutexF);
		//обработать полученный элемент
	}
}





int main()
{	
	int i;
	//инициализация мьютексов и семафоров
	pthread_mutex_init(&mutexD, NULL);
	pthread_mutex_init(&mutexF, NULL);
	sem_init(&empty_1, 0, z_1); //количество свободных ячеек равно 100
	sem_init(&full_1, 0, 0); //количество занятых ячеек равно 0
	sem_init(&empty_2, 0, z_2); //количество свободных ячеек равно 100
	sem_init(&full_2, 0, 0); //количество занятых ячеек равно 0
	cannons_generation();
	
	printf("Пушки первого города (%d):\n", z_1);
	for (int i = 0; i < z_1; i++)
	{
		printf("Пушка # %d: %d %d %d %d %d \n", i + 1, cannon_1[i][0], cannon_1[i][1], cannon_1[i][2], cannon_1[i][3], cannon_1[i][4]);
	}

	printf("Пушки второго города (%d):\n", z_2);
	for (int i = 0; i < z_2; i++)
	{
		printf("Пушка # %d: %d %d %d %d %d \n", i + 1, cannon_2[i][0], cannon_2[i][1], cannon_2[i][2], cannon_2[i][3], cannon_2[i][4]);
	}
	
	
	//запуск пушек первого города
	pthread_t *threadP = new pthread_t[z_1];
	for (i = 0; i<z_1; i++)
		pthread_create(&threadP[i], NULL, Cannons_1, NULL);
	//запуск пушек второго города
	pthread_t *threadC = new pthread_t[z_2];
	for (i = 0; i<z_2; i++)
		pthread_create(&threadC[i], NULL, Cannons_2, NULL);
	
	
	Cannons_2(NULL);

	int check = 0;
	for (int i = 0; i < z_1; i++)
	{
		if (cannon_1[i][0] >= 0)
		{
			check++;
			break;
		}
	}

	if (check > 0)
	{
		printf("У первого города имеются целые пушки");
	}

	check = 0;
	for (int i = 0; i < z_2; i++)
	{
		if (cannon_2[i][0] >= 0)
		{
			check++;
			break;
		}
	}

	if (check > 0)
	{
		printf("У второго города имеются целые пушки");
	}
	

}

