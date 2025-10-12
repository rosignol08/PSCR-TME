# Questions - TME 3 : Threads

Instructions : copiez vos réponses dans ce fichier (sous la question correspondante). A la fin de la séance, commitez vos réponses.

## Question 1.

```
cd build-release && ./TME3 ../WarAndPeace.txt freqstd && ./TME3 ../WarAndPeace.txt freqstdf && ./TME3 ../WarAndPeace.txt freq && check.sh *.freq


traces pour les 3 modes, invocation a check qui ne rapporte pas d'erreur
Total runtime (wall clock) : 2416 ms
Total runtime (wall clock) : 2500 ms
Total runtime (wall clock) : 2581 ms
All files are identical
```

## Question 2.

start vaut 0

end vaut file_size

Code des lambdas freqstdf :
```
[&](const std::string& word) {
                        total_words++;
                        um[word]++;
```
Code des lambdas freq :
```
[&](const std::string& word) {
    total_words++;
    hm.incrementFrequency(word);
}
```
Accès identifiés :
freqstdf :
"total_words" c'est l'accès en lecture et écriture par référence
"um (std::unordered_map)" c'est l'accès en modification par référence pour ajouter ou mettre à jour les compteurs de mots

freq :
"total_words" c'est l'accès en lecture et écriture par référence aussi.
"hm (HashMap)" : c'est l'accès en modification par référence avec la méthode incrementFrequency

## Question 3.

Continuez de compléter ce fichier avec vos traces et réponses.

...

## Question 4.
mt_naive :
Total runtime (wall clock) : 816 ms

## Question 5.
mt_hnaive :
Total runtime (wall clock) : 835 ms

## Question 6.
mt_atomic :
Total runtime (wall clock) : 820 ms

## Question 8.
mt_mutex :
Total runtime (wall clock) : 961 ms

## Question 10.
mt_hashes :
Total runtime (wall clock) : 858 ms
