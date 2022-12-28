# DocumentSearchFreq

This program takes in a search query from the user and returns the documents in p5docs that match the most. 
before the search query is taken the program enters all the words in a hashmap as nodes that will act as the head of a linked list of documents that contain that word.
If the word exists in all documents it is considered a stop word and is removed from the hashmap.
Once the scores are calculated they are appened to the search_scores.txt.
