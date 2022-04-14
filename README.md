# COP-4520-Spring-2022-Programming-Assignment-3

## Problem 1
To solve their problem, I created a linked list that has locks for each node.

### Insertion
Get the head, we'll call it `prev` and the next node, we'll call it `next` and lock them. Continuously go along, unlocking `prev`, maintaining `next` is still locked, setting `prev` to `next`, setting `next` to `next->next`, and locking `next`. By doing this, we maintain that no other servant can overtake me, since I always have a node locked in my operation. This also means that each servant only has to wait for anyone that is in front of them. Once we find where the value we are inserting goes between `prev` and `next`, insert it.

### Removal
Get the head, we'll call it `prev`, get the next node, we'll call it `curr`, get the next node after that, we'll call it `next`. Continuously go along, unlocking `prev`, maintaining `curr` is still locked, setting `prev` to `curr`, maintaining `next` is still locked, setting `curr` to `next`, setting `next` to `next->next`, and locking `next`. By the same argument in insertion, we guarantee no one can overtake us. Then we remove `curr` if it is the correct value to remove. Since we have the 2 nodes that surround it locked, no other servant is affected by removing this node.

## Problem 2
My procedure: Wait for all sensors to finish reading temperatures for the minute. Find the max and min value across all sensors for this minute. Store the difference between the max and min value. Insert the max value into a multiset of the last hour of max values and remove the oldest value that is an hour old. Do the same for min in a multiset of the last hour of min values. If it has been 10 minutes, see what the last 10 differences were and find the max. If it has been 1 hour, use the multiset to get the 5 min and 5 max in the last hour.

We have to accomplish all this in under 1 minute, which I think it should, since most of these instructions take really quickly to finish. Finding the min and max values of the sensors is a linear operation on the number of sensors. Storing the difference is a constant time operation. The multiset operations are the worst case scenario here, but I always maintain that the multisets only have the last hour of values in them, which means they hold at most 60 values. So each operation on them is at most log(60) which is ~6. And I do those operations 6 times. Getting the last 10 differences is linear on 10 since I only store the last 10. And the 5 min and 5 max are linear since I use the iterators of the multiset. So at most 2 linear operations of length 5. Since the number of operations is small, we will finish everything we need to account for well before the next readings.
