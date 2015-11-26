#! /usr/bin/python
# -*- coding:utf-8 -*-


from __future__ import division


if "__main__" == __name__:
    users = [
        {"id": 0, "name": "Hero"},
        {"id": 1, "name": "Dunn"},
        {"id": 2, "name": "Sue"},
        {"id": 3, "name": "Chi"},
        {"id": 4, "name": "Thor"},
        {"id": 5, "name": "Cliver"},
        {"id": 6, "name": "Hicks"},
        {"id": 7, "name": "Devin"},
        {"id": 8, "name": "Kate"},
        {"id": 9, "name": "Klein"},
    ]
    friendships = [
        (0, 1), (0, 2), (1, 2), (1, 3), (2, 3), (3, 4),
        (4, 5), (5, 6), (5, 7), (6, 8), (7, 8), (8, 9),
    ]

    for user in users:
        user["friends"] = []

    for i, j in friendships:
        users[i]["friends"].append(users[j])
        users[j]["friends"].append(users[i])

    def number_of_friends(user):
        """how many friends does _user_ have?"""
        return len(user["friends"])

    total_connections = sum(number_of_friends(user) for user in users)
    num_users = len(users)
    avg_connections = total_connections / num_users

    num_friends_by_id = [(user["id"], number_of_friends(user))
                         for user in users]
    print sorted(num_friends_by_id,
           key = lambda (user_id, num_friends): num_friends,
           reverse = True)
