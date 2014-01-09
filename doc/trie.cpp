#include <string>
#include <iostream>

using namespace std;

struct node {
    bool is_end;
    int prefix_count;
    struct node *child[26];
} *head;

void init(void)
{
    head = new node();
    head->prefix_count = 0;
    head->is_end = false;
}

void insert(string word)
{
    node *current = head;

    current->prefix_count++;
    for (unsigned int i = 0; i < word.length(); ++i) {
        int letter = (int)word[i] - (int)'a';

        if (NULL == current->child[letter]) {
            current->child[letter] = new node();
        }
        current->child[letter]->prefix_count++;
        current = current->child[letter];
    }

    current->is_end = true;
}

bool search(string word)
{
    node *current = head;

    for (unsigned int i = 0; i < word.length(); ++i) {
        int letter = (int)word[i] - (int)'a';

        if (NULL == current->child[letter]) {
            return false;
        }

        current = current->child[letter];
    }

    return current->is_end;
}

int words_with_prefix(string prefix)
{
    node *current = head;

    for (unsigned int i = 0; i < prefix.length(); ++i) {
        int letter = (int)prefix[i] - (int)'a';

        if (NULL == current->child[letter]) {
            return 0;
        } else {
            current = current->child[letter];
        }
    }

    return current->prefix_count;
}

int main(int argc, char *argv[])
{
    init();
    insert("abc");
    insert("abd");
    insert("abe");
    cout << words_with_prefix("ab") << endl;

    return 0;
}
