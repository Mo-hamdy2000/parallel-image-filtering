#include <iostream>
#include <chrono>
#include <vector>
#include <thread>

#define ROWS 256
#define COLS 256
#define FILTER_SIZE 128

void printArray(int array[ROWS][COLS]);
void sequentialFiltering(int array[ROWS][COLS], int filter[FILTER_SIZE][FILTER_SIZE], int filtered[ROWS][COLS]);
int calculateCell(int i, int j, int array[ROWS][COLS], int filter[FILTER_SIZE][FILTER_SIZE]) {
    double accum;
    for (int y = 0; y < FILTER_SIZE; y++)
        for (int x = 0; x < FILTER_SIZE; x++) {
            if (i + y - 1 >= 0 && i + y - 1 < ROWS && j + x - 1 >= 0 && j + x - 1 < COLS)
                accum += array[i + y - 1][j + x - 1] * filter[y][x] / 9;
        }
    return accum;
}

using namespace std;
using namespace std::chrono;

int main() {
    int randArray[ROWS][COLS];
    int filtered[ROWS][COLS];
    int filter[FILTER_SIZE][FILTER_SIZE];
    for (auto &i: randArray)
        for (int &j: i)
            j = rand() % 255;

    for (auto &i: filter)
        for (int &j: i)
            j = rand() % 11;

    auto start = high_resolution_clock::now();
    sequentialFiltering(randArray, filter, filtered);
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken by sequential function: " << duration.count() << " microseconds" << endl;

    const uint64_t num_threads = 8;
    auto block = [&](const int &id) -> void {
        const int chunk = ROWS / num_threads;
        const int lower = id * chunk;
        const int upper = std::min(lower + chunk, ROWS);

        for (int i = lower; i < upper; i++)
            for (int j = 0; j < COLS; j++)
                filtered[i][j] = calculateCell(i, j, randArray, filter);
    };

    auto cyclic = [&](const int &id) -> void {
        for (int i = id; i < ROWS; i += num_threads)
            for (int j = 0; j < COLS; j++)
                filtered[i][j] = calculateCell(i, j, randArray, filter);
    };

    vector<std::thread> threads;

    start = high_resolution_clock::now();
    for (uint64_t id = 0; id < num_threads; id++)
        threads.emplace_back(block, id);

    for (auto &thread: threads)
        thread.join();
    stop = high_resolution_clock::now();

    duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken by parallel block: " << duration.count() << " microseconds" << endl;

    threads.clear();

    start = high_resolution_clock::now();
    for (uint64_t id = 0; id < num_threads; id++)
        threads.emplace_back(cyclic, id);

    for (auto &thread: threads)
        thread.join();
    stop = high_resolution_clock::now();

    duration = duration_cast<microseconds>(stop - start);
    cout << "Time taken by parallel cyclic: " << duration.count() << " microseconds" << endl;

    return 0;
}

void sequentialFiltering(int array[ROWS][COLS], int filter[FILTER_SIZE][FILTER_SIZE], int filtered[ROWS][COLS]) {
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {
            filtered[i][j] = calculateCell(i, j, array, filter);
        }
    }
}

void printArray(int array[ROWS][COLS]) {
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++)
            cout << array[i][j] << "\t";
        cout << endl;
    }
}