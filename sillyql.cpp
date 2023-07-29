// Project Identifier: C0F4DFE8B340D81183C208F70F9D2D797908754D

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "TableEntry.h"
#include <algorithm>
#include "getopt.h"
#include <map>

using namespace std;

class Table {
    public:
        int nCols;
        string hashcname;
        vector<vector<TableEntry>> table;
        vector<string> col_types;
        vector<string> col_names;
        map<string,int> col_index;
        unordered_map<TableEntry, vector<int>> hash;
        map<TableEntry, vector<int>,less<TableEntry>> bst;

        Table() : nCols(0), hashcname(""), col_types(0), col_names(0), hash(0) {}
        Table(int nCols) : nCols(nCols), hashcname(""), col_types(0), col_names(0), hash(0) {}
};

static unordered_map<string, Table> database;
char op_compare;
int col_index_compare;
TableEntry* value_compare;
bool quietMode = false;

bool compare(vector<TableEntry> &t){
    if (op_compare == '='){
        return (*value_compare) == t[static_cast<size_t>(col_index_compare)];
    }
    else if (op_compare == '>'){
        return (*value_compare) < t[static_cast<size_t>(col_index_compare)];
    }
    else{
        return (*value_compare) > t[static_cast<size_t>(col_index_compare)];
    }
}

// Returns TableEntry type
TableEntry createTableEntry(string &type) {
    if (type == "bool") {
        bool x;
        cin >> x;
        return TableEntry(x);
    }
    else if (type == "double") {
        string in;
        cin >> in;
        return TableEntry(stod(in));
    }
    else if (type == "int") {
        int x;
        cin >> x;
        return TableEntry(x);
    }
    else if (type == "string") {
        string x;
        cin >> x;
        return TableEntry(x);
    }
    return TableEntry(0);
}


//returns TRUE if table is in database else false
bool foundTable(string &tname) {
    if(database.find(tname) != database.end())return true;
    else return false;
}

void regenerate(string &tname,string &cname) {
    auto foundCol = find(database[tname].col_names.begin(), database[tname].col_names.end(), cname);
    size_t index = static_cast<size_t>(foundCol - database[tname].col_names.begin());
    
    if(!database[tname].bst.empty()) {
        database[tname].bst.clear();
        for (size_t i = 0; i < database[tname].table.size(); i++) {
            database[tname].bst[database[tname].table[i][index]].emplace_back(i);
        }
    }
    
    else if(!database[tname].hash.empty()) {
        database[tname].hash.clear();
        for (size_t i = 0; i < database[tname].table.size(); i++) {
            database[tname].hash[database[tname].table[i][index]].emplace_back(i);
        }
    }
}

void create() {
    string tname,temp,cname;
    int tsize;
    
    cin >> tname >> tsize;
    
    // Error handling
    if (foundTable(tname)) {
        cout << "Error during CREATE: Cannot create already existing table " << tname << endl;
        getline(cin,temp);
        return;
    }
    
    database[tname] = Table(tsize);
    database[tname].col_types = vector<string>(static_cast<size_t>(tsize));
    database[tname].col_names = vector<string>(static_cast<size_t>(tsize));
    database[tname].hash.reserve(static_cast<size_t>(tsize));
    database[tname].table.reserve(static_cast<size_t>(tsize));
    
    for (int i = 0; i < tsize; i++) {
        cin >> database[tname].col_types[static_cast<size_t>(i)];
    }

    cout << "New table " << tname  << " with column(s) ";

    for (int i = 0; i < tsize; i++) {
        // column name
        cin >> cname;
    
        database[tname].col_names[static_cast<size_t>(i)] = cname;
        database[tname].col_index[cname] = i;

        cout << cname << " ";
    }

    cout << "created" << endl;
}

void insert() {
    string tname,temp;
    int nrows;
    
    cin >> tname;
    cin >> nrows;
    cin >> temp;
    
    // Error handling if table is not found
    if (!foundTable(tname)) {
        cout << "Error during INSERT: " << tname << " does not name a table in the database\n";

        // read all the rows
        // for (int i = 0; i < nrows; i++) {
        //     getline(cin, temp);
        // }
        return;
    }

    cout << "Added " << static_cast<unsigned long>(nrows) << " rows to " << tname << " from position " << static_cast<unsigned long>(database[tname].table.size()) << " to " << (static_cast<unsigned long>(nrows) + static_cast<unsigned long>(database[tname].table.size()) - 1) << endl;
    
    for (int j = 0; j < nrows; j++) {
        vector<TableEntry> record;
        for (int i = 0; i < database[tname].nCols; i++) {
            record.push_back(createTableEntry(database[tname].col_types[static_cast<size_t>(i)]));
        }
        database[tname].table.push_back(record);
    }

    if (database[tname].hashcname != "") {
        regenerate(tname, database[tname].hashcname);
    }
    
}

//PRINT
void print() {
    vector<string> columns;
    vector<int> col_indices;
    char op;
    string tname,seltype,cname,temp;
    int n;
    size_t pnrows = 0;
    
    cin >> tname;
    cin >> n;
    
    // Error handling table name is not found in database
    if (!foundTable(tname)) {
        cout << "Error during PRINT: " << tname << " does not name a table in the database\n";
        getline(cin, temp);
        return;
    }

    // read column names and indices
    for (int i = 0; i < n; i++) {
        cin >> cname;

        if (find(database[tname].col_names.begin(),database[tname].col_names.end(), cname) != database[tname].col_names.end()) {
            columns.push_back(cname);
            col_indices.push_back(database[tname].col_index[cname]);
        }
        else {
            cout << "Error during PRINT: " << cname << " does not name a column in " << tname << endl;
            getline(cin, temp);
            return;
        }
    }
    
    cin >> seltype;

    if (seltype == "ALL") {
        pnrows = database[tname].table.size();
        if(!quietMode){
            // printing column names
            for (int i = 0; i < n; i++) {
                cout << columns[static_cast<size_t>(i)] << " ";
            }
            cout << endl;

            for (size_t i = 0; i < database[tname].table.size(); i++) {
                for (int j = 0; j < n; j++) {
                    cout << database[tname].table[i][static_cast<size_t>(col_indices[static_cast<size_t>(j)])] << " ";
                }
                cout << endl;
            }
        }
    }
    
    else if (seltype == "WHERE") {
        cin >> cname >> op;

        auto col_iterator = find(database[tname].col_names.begin(), database[tname].col_names.end(), cname);
        
        if (col_iterator == database[tname].col_names.end()) {
            cout << "Error during PRINT: " << cname << " does not name a column in " << tname << endl;
            getline(cin, temp);
            return;
        }
        
        int col_index = database[tname].col_index[cname];
        
        TableEntry right_value = createTableEntry(database[tname].col_types[static_cast<size_t>(col_index)]);

        if(!quietMode){
        // printing column names
            for (int i = 0; i < n; i++) {
                cout << columns[static_cast<size_t>(i)] << " ";
            }
            cout << endl;
        }
        
        if (!database[tname].bst.empty() && database[tname].hashcname == cname){

            if (op == '=') {
                auto it = database[tname].bst.find(right_value);
                if (it != database[tname].bst.end()) {
                    pnrows += it->second.size();
                    if(!quietMode){
                        for (auto i : it->second) {
                            for (auto j: col_indices) {
                                cout << database[tname].table[static_cast<size_t>(i)][static_cast<size_t>(j)] << " ";
                            }
                            cout << endl;
                        }
                    }
                }
            }
            
            else if (op == '<') {

                auto it = database[tname].bst.lower_bound(right_value);
                for (auto it1 = database[tname].bst.begin(); it1 != it; it1++) {
                    pnrows += it1->second.size();
                    if(!quietMode){
                        for (auto i : it1->second) {
                            for (auto j : col_indices) {
                                cout << database[tname].table[static_cast<size_t>(i)][static_cast<size_t>(j)] << " ";
                            }
                            cout << "\n";
                        }
                    }
                }
            }
            
            else if (op == '>') {
                auto it = database[tname].bst.upper_bound(right_value);
                for (auto it1 = it; it1 != database[tname].bst.end(); it1++) {
                    pnrows += it1->second.size();
                    if(!quietMode){
                        for (auto i : it1->second) {
                            for (auto j : col_indices) {
                                cout << database[tname].table[static_cast<size_t>(i)][static_cast<size_t>(j)] << " ";
                            }
                            cout << "\n";
                        }
                    }
                }
            }
        }

        else if (quietMode && !database[tname].hash.empty() && database[tname].hashcname == cname && op == '=') {
            
            auto found = database[tname].hash.find(right_value);
            if (found != database[tname].hash.end()) {
                pnrows = found->second.size();
            }
        }

        else {
            vector<size_t> row;
            for (size_t i = 0; i < database[tname].table.size(); i++) {
                if (op == '=') {
                    if (database[tname].table[i][static_cast<size_t>(col_index)] == right_value) {
                        row.emplace_back(i);
                    }
                }
                else if(op == '>') {
                    if (database[tname].table[i][static_cast<size_t>(col_index)] > right_value) {
                        row.emplace_back(i);
                    }
                }
                else if(op == '<') {
                    if (database[tname].table[i][static_cast<size_t>(col_index)] < right_value) {
                        row.emplace_back(i);
                    }
                }
            }

            pnrows = row.size();
            if(!quietMode){
                for (auto i: row) {
                    for (auto j:col_indices) {
                        cout << database[tname].table[static_cast<size_t>(i)][static_cast<size_t>(j)] << " ";
                    }
                    cout << endl;
                }
            }
        }
    }
    cout << "Printed " << pnrows << " matching rows from " << tname << endl;
}

//DELETE
void deletefrom_table() {
    
    string tname,cname,temp;
    char op;
    
    cin >> tname >> temp >> cname >> op;
    
    if (!foundTable(tname)) {
        cout << "Error during DELETE: " << tname << " does not name a table in the database\n";
        return;
    }

    auto found = find(database[tname].col_names.begin(), database[tname].col_names.end(), cname);

    int col_index = int(found - database[tname].col_names.begin());
    size_t startPos = database[tname].table.size();
    
    if (found == database[tname].col_names.end()) {
        cout << "Error during DELETE: " << cname << " does not name a column in " << tname << endl;
        return;
    }

    else {
        TableEntry right_value = createTableEntry(database[tname].col_types[static_cast<size_t>(col_index)]);
        op_compare = op;
        value_compare = &right_value;
        col_index_compare = col_index;
        database[tname].table.erase(remove_if(database[tname].table.begin(),database[tname].table.end(),compare),database[tname].table.end());
    }
    
    if(database[tname].hashcname != "")regenerate(tname,database[tname].hashcname);
    
    cout << "Deleted " << startPos - database[tname].table.size() << " rows from " << tname << endl;
}


void generate(string &tname) {
    string type,temp,cname;
    cin >> type >> temp >> temp >> cname;
    
    database[tname].hashcname = cname;
    
    auto found = find(database[tname].col_names.begin(), database[tname].col_names.end(),cname);
    if (found == database[tname].col_names.end()) {
        cout << "Error during GENERATE: " << cname << " does not name a column in " << tname << endl;
        return;
    }
    if (!database[tname].bst.empty()) {
        database[tname].bst.clear();
    }
    if (!database[tname].hash.empty()) {
        database[tname].hash.clear();
    }

    int index = database[tname].col_index[cname];
    
    if (type == "hash"){
        for (size_t i = 0; i < database[tname].table.size(); i++) {
            database[tname].hash[database[tname].table[i][static_cast<size_t>(index)]].emplace_back(i);
        }
    }
    
    else {
        for (size_t i = 0; i < database[tname].table.size(); i++) {
            database[tname].bst[database[tname].table[i][static_cast<size_t>(index)]].emplace_back(i);
        }
    }
    cout << "Created " << type << " index for table " << tname << " on column " << cname << endl;
}

// JOIN
void join(){
    string tname1,tname2,temp,cname1,cname2;
    vector<string> cols;
    vector<int> col_table;
    int ncols = 0;

    cin >> tname1 >> temp >> tname2 >> temp >> cname1 >> temp >> cname2 >> temp >> temp >> ncols;

    if (!foundTable(tname1)) {
        cout << "Error during JOIN: " << tname1 << " does not name a table in the database\n"; 
        getline(cin, temp);
        return;
    }
    else if (!foundTable(tname2)) {
        cout << "Error during JOIN: " << tname2 << " does not name a table in the database\n";
        getline(cin, temp);
        return;
    }
    
    auto it1 = find(database[tname1].col_names.begin(), database[tname1].col_names.end(), cname1);
    auto it2 = find(database[tname2].col_names.begin(), database[tname2].col_names.end(), cname2);
    
    if (it1 == database[tname1].col_names.end()) {
        cout << "Error during JOIN: " << cname1 << " does not name a column in " << tname1 << endl;
        getline(cin, temp);
        return;
    }
    else if (it2 == database[tname2].col_names.end()) {
        cout << "Error during JOIN: " << cname2 << " does not name a column in " << tname2 << endl;
        getline(cin, temp);
        return;
    }

    unordered_map<TableEntry, vector<int>> tname2_map;
    string col,current_table;
    int table = 0,nrows = 0;
    
    for (int i = 0; i < ncols; i++) {
        cin >> col >> table;
        
        if(table == 1)current_table = tname1;
        else if(table == 2)current_table = tname2;
        else{
            cout << "Error during JOIN: Table number must be 1 or 2"<< endl;
            getline(cin, temp);
            return;
        }

        auto it = find(database[current_table].col_names.begin(),database[current_table].col_names.end(),col);
        
        if (it == database[current_table].col_names.end()) {
            cout << "Error during JOIN: " << col << " does not name a column in " << current_table << endl;
            getline(cin, temp);
            return;
        }
        else {
            cols.push_back(col);
            col_table.push_back(table);
        }
    }
    
    int index2 = database[tname2].col_index[cname2];
    for(size_t i = 0; i < database[tname2].table.size(); i++) {
        tname2_map[database[tname2].table[i][static_cast<size_t>(index2)]].emplace_back(i);
    }

    if(!quietMode){
        for (size_t i = 0; i < cols.size();i++) {
            cout << cols[i] << " ";
        }
        cout << "\n";
    }

    for(size_t i = 0; i < database[tname1].table.size(); i++) {
        auto it = tname2_map.find((database[tname1].table[i][static_cast<size_t>(database[tname1].col_index[cname1])]));
        if (it != tname2_map.end()) {
            for (auto k : it->second) {
                int index;
                if(!quietMode){
                    for (int j = 0; j < (int)cols.size(); j++) {
                        if (col_table[static_cast<size_t>(j)] == 1) {
                            index = database[tname1].col_index[cols[static_cast<size_t>(j)]];
                            cout << database[tname1].table[i][static_cast<size_t>(index)] << " ";
                        }
                        else {
                            index = database[tname2].col_index[cols[static_cast<size_t>(j)]];
                            cout << database[tname2].table[static_cast<size_t>(k)][static_cast<size_t>(index)] << " ";
                        }
                    }
                    cout << endl;
                }
                ++nrows;
            }
        }
    }

    cout << "Printed " << nrows << " rows from joining " << tname1 << " to " << tname2 << "\n";
}


int main(int argc,char** argv) {
    ios_base::sync_with_stdio(false);
    
    cin >> boolalpha;
    cout << boolalpha;
    
    string command;

    int optindex = 0;
    int option = 0;
    
    opterr = false;
    
    struct option comOpts[] = { {"quiet", no_argument, nullptr, 'q'},{"help", no_argument, nullptr, 'h'}};
    
    while ((option = getopt_long(argc, argv, "qh", comOpts, &optindex)) != -1) {
        switch (option) {
                case 'h':
                    cout << "Not implemented\n";
                    break;
                case 'q':
                    quietMode = true;
                    break;
        }
    }

    cout << "% ";
    cin >> command;
    
    while (command != "QUIT"){
        string temp;
        string tname;
        
        // COMMENT
        if (command.at(0) == '#') {
            getline(cin, temp);
        }
        
        // CREATE
        else if (command == "CREATE") {
            create();
        }

        // REMOVE
        else if (command == "REMOVE") {
            cin >> tname;

            bool found = foundTable(tname);

            if (found) {
                database.erase(database.find(tname));
                cout << "Table " << tname << " deleted" << endl;
            }
            else {
                cout << "Error during REMOVE: " << tname << " does not name a table in the database\n";
            }
        }
        
        // INSERT
        else if (command == "INSERT") {
            cin >> temp;
            insert();
        }

        // PRINT
        else if (command == "PRINT") {
            cin >> temp;
            print();
        }
        
        // DELETE
        else if (command == "DELETE") {
            cin >> temp;
            deletefrom_table();
        }

        // GENERATE
        else if (command == "GENERATE") {
            cin >> temp >> tname;
            bool found = foundTable(tname);
            
            if (!found) {
                cout << "Error during GENERATE: " << tname << " does not name a table in the database\n";
                getline(cin, temp);
            }
            else {
                generate(tname);
            }
        }

        // JOIN
        else if (command == "JOIN") {
            join();
        }

        else{
            cout << "Error: unrecognized command" << endl;
            getline(cin,temp);
        }

        cout << "% ";
        cin >> command;
     }
    
    if (command == "QUIT") {
        cout << "Thanks for being silly!" << endl;
    }

    return 0;
}