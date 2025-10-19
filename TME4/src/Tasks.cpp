#include "Tasks.h"
#include "util/ImageUtils.h"
#include "util/thread_timer.h"
#include <thread>
#include <sstream>

namespace pr {

TaskData* TASK_POISON = nullptr;

void treatImage(FileQueue& fileQueue, const std::filesystem::path& outputFolder) {
    pr::thread_timer timer;
    
    while (true) {
        std::filesystem::path file = fileQueue.pop();
        if (file == pr::FILE_POISON) break; //poison pill
        QImage original = pr::loadImage(file);
        if (!original.isNull()) {
            QImage resized = pr::resizeImage(original);
            std::filesystem::path outputFile = outputFolder / file.filename();
            pr::saveImage(resized, outputFile);
        }
    }

    // trace
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (treatImage): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}


void reader(FileQueue& fileQueue, ImageTaskQueue& imageQueue) {
    pr::thread_timer timer;
    while (true) {
        std::filesystem::path file = fileQueue.pop();
        if (file == pr::FILE_POISON) break; // poison pill
        //on cree un taskdata pour chaque image qu'on lis
        TaskData* td = new TaskData();
        td->chemin = file;
        td->image = pr::loadImage(file);
        imageQueue.push(td);
    }

    //quand c'est fini on envoie la pillule
    imageQueue.push(nullptr);

    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (reader): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}

void resizer(ImageTaskQueue& imageQueue, ImageTaskQueue& resizedQueue) {
    pr::thread_timer timer;
    while (true) {
        TaskData* td = imageQueue.pop();
        if (td == nullptr) {
            //la poison pill
            resizedQueue.push(nullptr);
            break;
        }

        //on resize l'image et on la remet dans notre TaskData
        if (!td->image.isNull()) {
            td->image = pr::resizeImage(td->image);
        }

        //pushhh
        resizedQueue.push(td);
    }
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (resizer): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
}

void saver(ImageTaskQueue& resizedQueue, const std::filesystem::path& outputFolder) {
    pr::thread_timer timer;
    while (true) {
        TaskData* td = resizedQueue.pop();
        if (td == nullptr) break; //poison pill

        if (!td->image.isNull()) {
            std::filesystem::path outputFile = outputFolder / td->chemin.filename();
            pr::saveImage(td->image, outputFile);
        }

        //faut delete le TaskData apres
        delete td;
    }
    std::stringstream ss;
    ss << "Thread " << std::this_thread::get_id() << " (saver): " << timer << " ms CPU" << std::endl;
    std::cout << ss.str();
};


} // namespace pr