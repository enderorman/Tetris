#include "Leaderboard.h"
#include "iostream"
#include "fstream"
#include "sstream"


int Leaderboard::getHighScore(){
    if (head_leaderboard_entry == nullptr){
        return 0;
    }
    return head_leaderboard_entry->score;
}
void Leaderboard::removeTail(){
    LeaderboardEntry* currentEntry = head_leaderboard_entry;
    while (currentEntry->next_leaderboard_entry->next_leaderboard_entry != nullptr){
        currentEntry = currentEntry->next_leaderboard_entry;
    }
    LeaderboardEntry* nextEntry = currentEntry->next_leaderboard_entry;
    currentEntry->next_leaderboard_entry = nullptr;
    delete nextEntry;
    currentSize--;
}
void Leaderboard::insert_new_entry(LeaderboardEntry * new_entry) {
    // TODO: Insert a new LeaderboardEntry instance into the leaderboard, such that the order of the high-scores
    //       is maintained, and the leaderboard size does not exceed 10 entries at any given time (only the
    //       top 10 all-time high-scores should be kept in descending order by the score).
    currentSize++;
    if (head_leaderboard_entry == nullptr){
        head_leaderboard_entry = new_entry;
        return;
    }
    LeaderboardEntry* currentEntry = head_leaderboard_entry;
    LeaderboardEntry* prev = nullptr;
    while (currentEntry != nullptr){
        if (new_entry->score > currentEntry->score){
            new_entry->next_leaderboard_entry = currentEntry;
            if (prev == nullptr){
                head_leaderboard_entry = new_entry;
                return;
            }
            prev->next_leaderboard_entry = new_entry;
            return;
        }
        prev = currentEntry;
        currentEntry = currentEntry->next_leaderboard_entry;
    }
    prev->next_leaderboard_entry = new_entry;
    if (currentSize > MAX_LEADERBOARD_SIZE){
        removeTail();
    }
}

void Leaderboard::write_to_file(const string& filename) {
    // TODO: Write the latest leaderboard status to the given file in the format specified in the PA instructions
    LeaderboardEntry* currentEntry = head_leaderboard_entry;
    ofstream outfile(filename);
    while (currentEntry != nullptr){
        string playerName = currentEntry->player_name;
        int score = currentEntry->score;
        time_t lastPlayed = currentEntry->last_played;
        outfile << score << " " << lastPlayed << " " << playerName << "\n";
        currentEntry = currentEntry->next_leaderboard_entry;
    }
    outfile.close();
}

void Leaderboard::read_from_file(const string& filename) {
    // TODO: Read the stored leaderboard status from the given file such that the "head_leaderboard_entry" member
    //       variable will point to the highest all-times score, and all other scores will be reachable from it
    //       via the "next_leaderboard_entry" member variable pointer.
    ifstream input(filename);
    string line;
    if (input.is_open()){
        while (getline(input, line)){
            istringstream ss(line);
            LeaderboardEntry* playerEntry;
            unsigned long score;
            time_t lastPlayed;
            string playerName;
            ss >> score >> lastPlayed >> playerName;
            playerEntry = new LeaderboardEntry(score, lastPlayed, playerName);
            insert_new_entry(playerEntry);
        }
    }
}


void Leaderboard::print_leaderboard() {
    // TODO: Print the current leaderboard status to the standard output in the format specified in the PA instructions
    cout << "Leaderboard" << "\n";
    cout << "-----------" << "\n";
    LeaderboardEntry* currentEntry = head_leaderboard_entry;
    int order = 1;
    while (currentEntry != nullptr){
        string playerName = currentEntry->player_name;
        unsigned long score = currentEntry->score;
        time_t lastPlayed = currentEntry->last_played;
        tm* ptm = localtime(&lastPlayed);
        char buffer[32];
        std::strftime(buffer, 32, "%H:%M:%S/%d.%m.%Y", ptm);
        cout << order << ". " << playerName << " " << score << " " << buffer << "\n";
        currentEntry = currentEntry->next_leaderboard_entry;
        order++;
    }
}

Leaderboard::~Leaderboard() {
    // TODO: Free dynamically allocated memory used for storing leaderboard entries
    LeaderboardEntry* currentEntry = head_leaderboard_entry;
    while(currentEntry!= nullptr){
        LeaderboardEntry* temp=currentEntry;
        currentEntry=currentEntry->next_leaderboard_entry;
        delete temp;

    }
    currentEntry= nullptr;

}


