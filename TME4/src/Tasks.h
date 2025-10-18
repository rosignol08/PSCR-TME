#ifndef TASKS_H
#define TASKS_H

#include <QImage>
#include <filesystem>
#include "BoundedBlockingQueue.h"

namespace pr {

using FileQueue = BoundedBlockingQueue<std::filesystem::path>;

const std::filesystem::path FILE_POISON{};

// load/resize/save 
void treatImage(FileQueue& fileQueue, const std::filesystem::path& outputFolder);

using ImageTaskQueue = BoundedBlockingQueue<TaskData*>;

struct TaskData {
    QImage image;
    std::filesystem::path chemin;
};

// or
//using ImageTaskQueue = BoundedBlockingQueue<TaskData>;

// TODO
void reader(FileQueue& fileQueue, ImageTaskQueue& imageQueue);
void resizer(ImageTaskQueue& imageQueue, ImageTaskQueue& resizedQueue);
void saver(ImageTaskQueue& resizedQueue, const std::filesystem::path& outputFolder);

} // namespace pr

#endif // TASKS_H
/*
reader : entrée un nom de fichier, sortie un QImage : décode et charge en mémoire
resizer : entrée un QImage, sortie un QImage : redimensionne pour rendre plus petit
writer : entrée un QImage, un dossier, un nom : sauvegarde l'image dans dossier/nom
*/

//définir (dans Tasks.h) une struct TaskData qui décrit les données à
//véhiculer et permet de fxer le paramètre template de BoundedBlockingQueue. 
//Attention, vu que la dernière étape a besoin du nom de fichier pour sauvegarder, 
//il faudra a priori transmettre ce nom avec l'image elle même.
//Quels champs doit contenir TaskData ? Devrions-nous utiliser un pointeur,
//une référence ou un stockage direct pour l'image 3 ?
