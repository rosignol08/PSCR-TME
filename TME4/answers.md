# TME4 Answers

Tracer vos expériences et conclusions dans ce fichier.

Le contenu est indicatif, c'est simplement la copie rabotée d'une IA, utilisée pour tester une version de l'énoncé.
On a coupé ses réponses complètes (et souvent imprécises voire carrément fausses, deadlocks etc... en Oct 2025 les LLM ont encore beaucoup de mal sur ces questions, qui demandent du factuel et des mesures, et ont de fortes tendances à inventer).
Cependant on a laissé des indications en particulier des invocations de l'outil possibles, à adapter à votre code.

## Question 1: Baseline sequential

### Measurements (Release mode)

**Resize + pipe mode:**
```
./build/TME4 -m resize -i input_images -o output_images

Thread 125991718549824 (main): 5164 ms CPU
Total runtime (wall clock): 5477 ms
Memory usage: Resident: 45.2 MB, Peak: 138 MB
Total CPU time across all threads: 5164 ms


./build/TME4 -m pipe -i input_images -o output_images

Thread 138258191218368 (treatImage): 5153 ms CPU
Thread 138258238728512 (main): 3 ms CPU
Total runtime (wall clock): 5468 ms
Memory usage: Resident: 59.6 MB, Peak: 145 MB
Total CPU time across all threads: 5156 ms

```
Le CPU est pas forcément utilisé tout le temps donc cpu time != time
le pic de ram est celui de la plus grosse image.


## Question 2: Steps identification

I/O-bound: 16, 19, 20 fichier Task.cpp et parseOptions dans main.cpp
CPU-bound: la partie pipe dans main()

parallelisable a priori ? Oui.

## Question 3: BoundedBlockingQueue analysis

Dans tout les cas ça mène à un deadlock
cv.wait capture la longueur de la queue et check si elle est bien plus petite que la taille max.

## Question 4: Pipe mode study

FILE_POISON sert à tuer un thread en lui donnant pas une image à manger.

Order/invert :on peut changer 2 et 3 mais pas les autres car si on donne le poison avant de chercher les fichier les thread meurt avant donc c'est inutile et si on les join avant de les tuer c'est problématique parce qu'ils continuent de tourner.


## Question 5: Multi-thread pipe_mt

Implement pipe_mt mode with multiple worker threads.

voir le code

Measurements:
- N=1: 
```
./build/TME4 -m pipe_mt -n 1 -i input_images -o output_images
Thread 134541018658496 (treatImage): 5165 ms CPU
Thread 134541073262912 (main): 2 ms CPU
Total runtime (wall clock): 5520 ms
Memory usage: Resident: 59.6 MB, Peak: 145 MB
Total CPU time across all threads: 5167 ms
```
- N=2: 
```
./build/TME4 -m pipe_mt -n 2 -i input_images -o output_images
Thread 138075531372224 (treatImage): 2699 ms CPU
Thread 138075539764928 (treatImage): 2670 ms CPU
Thread 138075587275072 (main): 4 ms CPU
Total runtime (wall clock): 2907 ms
Memory usage: Resident: 115 MB, Peak: 241 MB
Total CPU time across all threads: 5373 ms
```
- N=4: 
```
./build/TME4 -m pipe_mt -n 4 -i input_images -o output_images
Thread 132630529959616 (treatImage): 1441 ms CPU
Thread 132630538352320 (treatImage): 1430 ms CPU
Thread 132630513174208 (treatImage): 1505 ms CPU
Thread 132630521566912 (treatImage): 1633 ms CPU
Thread 132630577756480 (main): 4 ms CPU
Total runtime (wall clock): 2036 ms
Memory usage: Resident: 151 MB, Peak: 391 MB
Total CPU time across all threads: 6013 ms
```
- N=8: 
```
./build/TME4 -m pipe_mt -n 8 -i input_images -o output_images
Thread 136916649363136 (treatImage): 616 ms CPU
Thread 136916624185024 (treatImage): 661 ms CPU
Thread 136916666148544 (treatImage): 888 ms CPU
Thread 136916632577728 (treatImage): 665 ms CPU
Thread 136916657755840 (treatImage): 728 ms CPU
Thread 136916615792320 (treatImage): 758 ms CPU
Thread 136916674541248 (treatImage): 929 ms CPU
Thread 136916640970432 (treatImage): 778 ms CPU
Thread 136916716587328 (main): 2 ms CPU
Total runtime (wall clock): 1785 ms
Memory usage: Resident: 233 MB, Peak: 523 MB
Total CPU time across all threads: 6025 ms
```

Best: 8 ?

## Question 6: TaskData struct

```cpp
struct TaskData {
... choix adopté
};
```

Fields: QImage ??? for the image data, ...

Use ??? for QImage, because ...

TASK_POISON: ...def...

## Question 7: ImageTaskQueue typing

pointers vs values

Choose BoundedBlockingQueue<TaskData???> as consequence

## Question 8: Pipeline functions

Implement reader, resizer, saver in Tasks.cpp.

mt_pipeline mode: Creates threads for each stage, with configurable numbers.

Termination: Main pushes the appropriate number of poisons after joining the previous stage.

Measurements: 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images
...
```


## Question 9: Configurable parallelism

Added nbread, nbresize, nbwrite options.


Timings:
- 1/1/1 (default): 
```
./build/TME4 -m mt_pipeline -i input_images -o output_images
...
```
- 1/4/1: 
```
./build/TME4 -m mt_pipeline --nbread 1 --nbresize 4 --nbwrite 1 -i input_images -o output_images
```

- 4/1/1: 
```
./build/TME4 -m mt_pipeline --nbread 4 --nbresize 1 --nbwrite 1 -i input_images -o output_images
```
... autres configs

Best config: 
interprétation

## Question 10: Queue sizes impact


With size 1: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 1 -i input_images -o output_images
...
```

With size 100: 
```
./build/TME4 -m pipe_mt -n 2 --queue-size 100 -i input_images -o output_images
...
```

impact

Complexity: 


## Question 11: BoundedBlockingQueueBytes

Implemented with byte limit.

mesures

## Question 12: Why important

Always allow push if current_bytes == 0, ...

Fairness: ...

## Bonus

