// Given a sorted linked list, delete all nodes that have duplicate numbers, leaving only distinct numbers from the original list.

// For example,
// Given 1->2->3->3->4->4->5, return 1->2->5.
// Given 1->1->1->2->3, return 2->3.

// Subscribe to see which companies asked this question


/**

 * Definition for singly-linked list.

 * struct ListNode {

 *     int val;

 *     ListNode *next;

 *     ListNode(int x) : val(x), next(NULL) {}

 * };

 */

class Solution {

public:
ListNode* deleteDuplicates(ListNode* head) {
    ListNode *stick, **p, *n;

    if (NULL == head) {
        return NULL;
    }

    // 确定stick
    stick = head;
    p = &head->next;
    while (1) {
        if (NULL == *p) {
            if (*p == stick->next) {
                return stick;
            } else {
                return NULL;
            }
        }

        if ((*p)->val != stick->val) {
            if (*p != stick->next) {
                stick = *p;
                p = &stick->next;
                continue;
            } else {
                n = (*p)->next;
                break;
            }
        }

        p = &((*p)->next);
    }

    // stick will not change any more

    while (1) {
        if (NULL == n) {
            if ((*p)->next != n) {
                *p = NULL;
            }

            break;
        }

        if (n->val != (*p)->val) {
            if (n != (*p)->next) {
                *p = n;
            } else {
                p = &((*p)->next);
            }
        }

        n = n->next;
    }

    return stick;
}
};
