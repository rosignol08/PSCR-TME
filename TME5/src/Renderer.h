#pragma once

#include<vector>
#include<thread>

#include "Scene.h"
#include "Image.h"
#include "Ray.h"
#include "Pool.h"
#include "Job.h"

namespace pr {
class PixelJob : 
    public Job {
	const Scene& scene;//const psk on modif pas la scene
	Image& img; //pas const psk on modifie les pixels
	int x;
	int y;

public:
	PixelJob(const Scene& scene, Image& img, int x, int y)
		: scene(scene), img(img), x(x), y(y) {}

	void run() override {
		const Scene::screen_t& screen = scene.getScreenPoints();
		const auto& screenPoint = screen[y][x];
		Ray ray(scene.getCameraPos(), screenPoint);
		int targetSphere = scene.findClosestInter(ray);

		if (targetSphere == -1) {
			// pixel = couleur de fond (ou rien à faire)
			return;
		}

		// calcul de la couleur
		const Sphere& obj = scene.getObject(targetSphere);
		Color finalColor = scene.computeColor(obj, ray);

		// mettre à jour le pixel
		img.pixel(x, y) = finalColor;
	}
};

class LineJob : public Job {
    const Scene& scene;
    Image& img;
    int y;
public:
    LineJob (const Scene& scene, Image& img, int y)
    : scene(scene), img(img), y(y) {}
    void run() override {
        const Scene::screen_t& screen = scene.getScreenPoints(); //la scene
        int width = scene.getWidth();

        for (int x = 0; x < width; ++x) {//pr chaque lignes
            const auto& screenPoint = screen[y][x];
            Ray ray(scene.getCameraPos(), screenPoint);

            int targetSphere = scene.findClosestInter(ray);//l'obj le plus proche
            if (targetSphere == -1) continue;

            const Sphere& obj = scene.getObject(targetSphere);
            Color finalColor = scene.computeColor(obj, ray);
            img.pixel(x, y) = finalColor;
        }
    }
};
// Classe pour rendre une scène dans une image
class Renderer {
public:
    // Rend la scène dans l'image
    void render(const Scene& scene, Image& img) {
        // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        const Scene::screen_t& screen = scene.getScreenPoints();

        // pour chaque pixel, calculer sa couleur
        for (int x = 0; x < scene.getWidth(); x++) {
            for (int y = 0; y < scene.getHeight(); y++) {
                // le point de l'ecran par lequel passe ce rayon
                auto& screenPoint = screen[y][x];
                // le rayon a inspecter
                Ray ray(scene.getCameraPos(), screenPoint);

                int targetSphere = scene.findClosestInter(ray);

                if (targetSphere == -1) {
                    // keep background color
                    continue;
                } else {
                    const Sphere& obj = scene.getObject(targetSphere);
                    // pixel prend la couleur de l'objet
                    Color finalcolor = scene.computeColor(obj, ray);
                    // mettre a jour la couleur du pixel dans l'image finale.
                    img.pixel(x, y) = finalcolor;
                }
            }
        }
    }
/*
    std::vector <std::thread> travailleurs;
        // 2. Start the workers thread
        for (int i = 0; i < opts.num_threads; ++i){
            travailleurs.emplace_back(pr::treatImage, std::ref(fileQueue), std::ref(opts.outputFolder));
            //std::thread worker(pr::treatImage, std::ref(fileQueue), std::ref(opts.outputFolder));
        }

        // 5. Join the worker thread
        
        for (int i = 0; i < opts.num_threads; ++i){
            travailleurs[i].join();
        }
*/

    void renderThreadPerPixel(const Scene& scene, Image& img){
        // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        std::vector<std::thread> travailleurs;
        // pour chaque pixel, calculer sa couleur
        auto render_lambda = [] (const Scene& scene, Image& img, int x, int y){
            const Scene::screen_t& screen = scene.getScreenPoints();
            //for (; x < scene.getWidth(); x++) {
            //    for (; y < scene.getHeight(); y++) {
                    // le point de l'ecran par lequel passe ce rayon
                    auto& screenPoint = screen[y][x];
                    // le rayon a inspecter
                    Ray ray(scene.getCameraPos(), screenPoint);

                    int targetSphere = scene.findClosestInter(ray);

                    if (targetSphere == -1) {
                        // keep background color
                        //continue;
                    } else {
                        const Sphere& obj = scene.getObject(targetSphere);
                        // pixel prend la couleur de l'objet
                        Color finalcolor = scene.computeColor(obj, ray);
                        // mettre a jour la couleur du pixel dans l'image finale.
                        img.pixel(x, y) = finalcolor;
                    }
//                }
//            }
        };
        int x = 0;
        int y = 0;
        int nb_thread = scene.getHeight()*scene.getWidth();
        for (int i = 0; i < nb_thread; ++i){
            travailleurs.emplace_back(render_lambda, std::cref(scene), std::ref(img), x, y);
        }
        for (int i = 0; i < nb_thread; ++i){
            travailleurs[i].join();
        }
        
    }
    void renderThreadPerRow(const Scene& scene, Image& img){
        // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        std::vector<std::thread> travailleurs;
        // pour chaque pixel, calculer sa couleur
        auto render_lambda_row = [] (const Scene& scene, Image& img, int x, int y){
            const Scene::screen_t& screen = scene.getScreenPoints();
            for (; x < scene.getWidth(); x++) {
                // le point de l'ecran par lequel passe ce rayon
                auto& screenPoint = screen[y][x];
                // le rayon a inspecter
                Ray ray(scene.getCameraPos(), screenPoint);

                int targetSphere = scene.findClosestInter(ray);

                if (targetSphere == -1) {
                    // keep background color
                    continue;
                } else {
                    const Sphere& obj = scene.getObject(targetSphere);
                    // pixel prend la couleur de l'objet
                    Color finalcolor = scene.computeColor(obj, ray);
                    // mettre a jour la couleur du pixel dans l'image finale.
                    img.pixel(x, y) = finalcolor;
                }
            }
        };
        int x = 0;
        //int y = 0;
        int nb_thread = scene.getHeight();
        for (int i = 0; i < nb_thread; ++i){
            travailleurs.emplace_back(render_lambda_row, std::cref(scene), std::ref(img), x, i);
        }
        for (int i = 0; i < nb_thread; ++i){
            travailleurs[i].join();
        }   
    }
    void renderThreadManual(const Scene& scene, Image& img, int nbthread){
        // les points de l'ecran, en coordonnées 3D, au sein de la Scene.
        // on tire un rayon de l'observateur vers chacun de ces points
        std::vector<std::thread> travailleurs;
        // pour chaque pixel, calculer sa couleur
        auto render_lambda_row = [] (const Scene& scene, Image& img, int y_debut, int y_fin){
            const Scene::screen_t& screen = scene.getScreenPoints();
            for (int y = y_debut; y < y_fin; ++y) {
                for (int x = 0; x < scene.getWidth(); x++) {
                    // le point de l'ecran par lequel passe ce rayon
                    auto& screenPoint = screen[y][x];
                    // le rayon a inspecter
                    Ray ray(scene.getCameraPos(), screenPoint);

                    int targetSphere = scene.findClosestInter(ray);

                    if (targetSphere == -1) {
                        // keep background color
                        continue;
                    } else {
                        const Sphere& obj = scene.getObject(targetSphere);
                        // pixel prend la couleur de l'objet
                        Color finalcolor = scene.computeColor(obj, ray);
                        // mettre a jour la couleur du pixel dans l'image finale.
                        img.pixel(x, y) = finalcolor;
                    }
                }
            }
        };
        int y_max = scene.getHeight();
        int nb_collones = y_max/nbthread; //division sans reste
        int reste = y_max%nbthread; //le reste donné au dernier thread
        int y_debut = 0;
        int y_fin;
        //les reste premier thread qui font 1 ligne en plus
        for (int i = 0; i < nbthread; ++i) {
            y_fin = y_debut + nb_collones + (i < reste ? 1 : 0);
            travailleurs.emplace_back(render_lambda_row, std::cref(scene), std::ref(img), y_debut, y_fin);
                
        }
        for (int i = 0; i < nbthread; ++i){
            travailleurs[i].join();
        }
    }
    void renderPoolPixel(const Scene& scene, Image& img, int nbThreads) {
    //taille queue ex nbPixels / 10
    int queueSize = (scene.getWidth() * scene.getHeight()) / 10;
    if (queueSize < 10) queueSize = 10;

    Pool pool(queueSize);
    pool.start(nbThreads);

    //envoie des job à chaque pixel
    for (int y = 0; y < scene.getHeight(); ++y) {
        for (int x = 0; x < scene.getWidth(); ++x) {
            pool.submit(new pr::PixelJob(scene, img, x, y));
        }
    }
    pool.stop();
    }

    void renderPoolRow(const Scene& scene, Image& img, int nbThreads) {
    //taille queue = nb lignes
    int queueSize = scene.getHeight();
    if (queueSize < 10) queueSize = 10;

    pr::Pool pool(queueSize);
    pool.start(nbThreads);

    //envoie des job à chaque ligne
    for (int y = 0; y < scene.getHeight(); ++y) {
        pool.submit(new pr::LineJob(scene, img, y));
    }

    pool.stop();
    }
};
  

} // namespace pr