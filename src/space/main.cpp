#include <vector>
#include <cstddef>
#include <iostream>
#include <GL/glew.h>
#include <c3ga/Mvec.hpp>
#include <glimac/glm.hpp>
#include <glimac/Cube.hpp>
#include <glimac/Tore.hpp>
#include <glimac/Image.hpp>
#include <glimac/Sphere.hpp>
#include <glimac/common.hpp>
#include <glimac/Program.hpp>
#include <glimac/FilePath.hpp>
#include <glimac/Geometry.hpp>
#include <../include/space/SkyBox.hpp>
#include <glimac/SDLWindowManager.hpp>
#include <../include/space/Texture.hpp>
#include <../include/glimac/FreeflyCamera.hpp>
#include <../include/space/Transformation.hpp>

using namespace glimac;
using namespace glm;

const std::string TEXTURE_DIR = "../assets/textures";
const GLuint VERTEX_ATTR_POSITION = 0;
const GLuint VERTEX_ATTR_NORMAL = 1;
const GLuint VERTEX_ATTR_TEXCOORD = 2;

glm::mat4 drawPlanet(Sphere & sphere, TexProgram & program, Texture & tex, GLuint tex_planet, SDLWindowManager & windowManager, glm::mat4 & globalMVMatrix, 
                     glm::mat4 & ProjMatrix, glm::vec3 & rotateGlobal, glm::vec3 & translate, glm::vec3 & scale, glm::vec3 & rotate, float speed) {
    program.m_Program.use();
    glUniform1i(program.uTexture, 0);
    glm::mat4 MVMatrix = glm::rotate(globalMVMatrix, windowManager.getTime()*speed, rotateGlobal);
    MVMatrix = glm::translate(MVMatrix, translate);
    MVMatrix = glm::scale(MVMatrix, scale);
    MVMatrix = glm::rotate(MVMatrix, windowManager.getTime(), rotate);
    // Specify the value of a uniform variable for the current program object
    glUniformMatrix4fv(program.uMVMatrix, 1, GL_FALSE, glm::value_ptr(MVMatrix));
    glUniformMatrix4fv(program.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MVMatrix))));
    glUniformMatrix4fv(program.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * MVMatrix));
    tex.activeAndBindTexture(GL_TEXTURE0, tex_planet);
    glDrawArrays(GL_TRIANGLES, 0, sphere.getVertexCount());
    glActiveTexture(GL_TEXTURE0);
    tex.activeAndBindTexture(GL_TEXTURE0, 0);
    glUniform1i(program.uTexture, 0);

    return MVMatrix;
}

Tore initTore(float ri, float re, GLuint & vbo_tore, GLuint & vao_tore) {
    // Trajectoire
    Tore tore(ri, re, 72, 36);

    glGenBuffers(1, &vbo_tore);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tore);

    glBufferData(GL_ARRAY_BUFFER, tore.getVertexCount() * sizeof(ShapeVertex), tore.getDataPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &vao_tore);
    glBindVertexArray(vao_tore);
    
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_tore);

    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, position)));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return tore;
}

void drawTore(Tore & tore, GLuint & vao_tore, TexProgram & saturneProgram, glm::mat4 & globalMVMatrix, SDLWindowManager & windowManager, GLuint texture, 
              glm::mat4 & ProjMatrix, glm::vec3 & translateSaturne) {
    glBindVertexArray(vao_tore);

    saturneProgram.m_Program.use();
    glm::mat4 toreMVMatrix = glm::rotate(globalMVMatrix, windowManager.getTime() * 0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
    toreMVMatrix = glm::rotate(toreMVMatrix, 80.0f, glm::vec3(1, 0, 0));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(saturneProgram.uTexture, 0);
    glUniformMatrix4fv(saturneProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(toreMVMatrix));
    glUniformMatrix4fv(saturneProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * toreMVMatrix));
    glUniformMatrix4fv(saturneProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(toreMVMatrix))));
    glDrawArrays(GL_TRIANGLES, 0, tore.getVertexCount());

    glBindVertexArray(0);
}

void freeVboVao(GLuint & vbo, GLuint & vao) {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

int main(int argc, char** argv) {
	int width_windows = 1350;
    int height_windows = 700;
    float ratio_h_w = (float)width_windows / (float)height_windows;
    // Initialize SDL and open a window
    SDLWindowManager windowManager(width_windows, height_windows, "Systeme solaire");

    // Initialize glew for OpenGL3+ support
    GLenum glewInitError = glewInit();
    if (GLEW_OK != glewInitError) {
        std::cerr << glewGetErrorString(glewInitError) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "OpenGL Version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version : " << glewGetString(GLEW_VERSION) << std::endl;

    /*********************************
     * HERE SHOULD COME THE INITIALIZATION CODE
     *********************************/
    FilePath applicationPath(argv[0]);
    TexProgram earthProgram(applicationPath);
    TexProgram sunProgram(applicationPath);
    TexProgram moonProgram(applicationPath);
    TexProgram mercureProgram(applicationPath);
    TexProgram venusProgram(applicationPath);
    TexProgram marsProgram(applicationPath);
    TexProgram jupiterProgram(applicationPath);
    TexProgram saturneProgram(applicationPath);
    TexProgram uranusProgram(applicationPath);
    TexProgram neptuneProgram(applicationPath);
    TexProgram callistoProgram(applicationPath);
    Skytext skytex(applicationPath);

    // Sphere pour les planetes
    Sphere sphere(1, 32, 16); // rayon = 1, latitude = 32, longitude = 16
    // Tore pour l'anneau de Saturne
    Tore tore(0.5, 3, 72, 36); // rayon_interne = 0.1, rayon_externe = 1

    // Trajectoire de Mercure
    GLuint vbo_mercure, vao_mercure;
    Tore TrajectoireMercure = initTore(0.2, 16.5, vbo_mercure, vao_mercure);
    // Trajectoire de Venus
    GLuint vbo_venus, vao_venus;
    Tore TrajectoireVenus = initTore(0.2, 24.5, vbo_venus, vao_venus);
    // Trajectoire de la Terre
    GLuint vbo_terre, vao_terre;
    Tore TrajectoireTerre = initTore(0.2, 30.5, vbo_terre, vao_terre);
    // Trajectoire de Mars
    GLuint vbo_mars, vao_mars;
    Tore TrajectoireMars = initTore(0.2, 42, vbo_mars, vao_mars);
    // Trajectoire de Jupiter
    GLuint vbo_jupiter, vao_jupiter;
    Tore TrajectoireJupiter = initTore(0.2, 63, vbo_jupiter, vao_jupiter);
    // Trajectoire de Saturne
    GLuint vbo_saturne, vao_saturne;
    Tore TrajectoireSaturne = initTore(0.2, 88, vbo_saturne, vao_saturne);
    // Trajectoire de Uranus
    GLuint vbo_uranus, vao_uranus;
    Tore TrajectoireUranus = initTore(0.2, 108, vbo_uranus, vao_uranus);
    // Trajectoire de Neptune
    GLuint vbo_neptune, vao_neptune;
    Tore TrajectoireNeptune = initTore(0.2, 135, vbo_neptune, vao_neptune);
    
    /* SkyBox */
    float size_cube = 1;
    Cube cubeSkybox(size_cube);
    GLsizei count_vertex_skybox = cubeSkybox.getVertexCount();
    const ShapeVertex*  Datapointeur_skybox = cubeSkybox.getDataPointer();
    ShapeVertex verticesSkybox[count_vertex_skybox];
    for (auto i = 0; i < count_vertex_skybox; i++) { // Skybox
        verticesSkybox[i] = *Datapointeur_skybox;
        verticesSkybox[i].position.x -= 0.5;
        verticesSkybox[i].position.y -= 0.5;
        verticesSkybox[i].position.z += 0.5;
        Datapointeur_skybox++;
    }
    SkyBox skybox(count_vertex_skybox, verticesSkybox);

    GLuint texSpatial;
    // Texture Spatial Skybox
    std::vector<std::string> facesGalaxy {
        TEXTURE_DIR + "/etoiles/right.png",
        TEXTURE_DIR + "/etoiles/left.png",
        TEXTURE_DIR + "/etoiles/top1.png",
        TEXTURE_DIR + "/etoiles/bottom.png",
        TEXTURE_DIR + "/etoiles/front.png",
        TEXTURE_DIR + "/etoiles/back.png"
    };
    //Binding de la texture Spatial
    texSpatial = skybox.loadCubemap(facesGalaxy);
    float distRendu = 5000.0f;
    /***************************/

    /* Textures planetes */
    Texture tex;
    std::unique_ptr<Image> SunMap = loadImage("../assets/textures/SunMap.jpg");
    std::unique_ptr<Image> MoonMap = loadImage("../assets/textures/MoonMap.jpg");
    std::unique_ptr<Image> CloudMap = loadImage("../assets/textures/CloudMap.jpg");
    std::unique_ptr<Image> EarthMap = loadImage("../assets/textures/EarthMap.jpg");
    std::unique_ptr<Image> MercureMap = loadImage("../assets/textures/Mercure.jpg");
    std::unique_ptr<Image> VenusMap = loadImage("../assets/textures/Venus.jpg");
    std::unique_ptr<Image> MarsMap = loadImage("../assets/textures/Mars.jpg");
    std::unique_ptr<Image> JupiterMap = loadImage("../assets/textures/Jupiter.jpg");
    std::unique_ptr<Image> SaturneMap = loadImage("../assets/textures/Saturne.jpg");
    std::unique_ptr<Image> UranusMap = loadImage("../assets/textures/Uranus.jpg");
    std::unique_ptr<Image> NeptuneMap = loadImage("../assets/textures/Neptune.jpg");
    std::unique_ptr<Image> CallistoMap = loadImage("../assets/textures/Callisto.jpg");
    if (SunMap == NULL || MoonMap == NULL || CloudMap == NULL || EarthMap == NULL
        || MercureMap == NULL || VenusMap == NULL || MarsMap == NULL || JupiterMap == NULL
        || SaturneMap == NULL || UranusMap == NULL || NeptuneMap == NULL || CallistoMap == NULL) {
        std::cerr << "Une des textures n'a pas pu etre chargée. \n" << std::endl;
        exit(0);
    }
    GLuint texture[12];
    glGenTextures(11, texture);
    tex.firstBindTexture(SunMap, texture[0]); //Binding de la texture SunMap
    tex.firstBindTexture(MoonMap, texture[1]); //Binding de la texture MoonMap
    tex.firstBindTexture(CloudMap, texture[2]); //Binding de la texture CloudMap
    tex.firstBindTexture(EarthMap, texture[3]); //Binding de la texture EarthMap
    tex.firstBindTexture(MercureMap, texture[4]); //Binding de la texture MercureMap
    tex.firstBindTexture(VenusMap, texture[5]); //Binding de la texture VenusMap
    tex.firstBindTexture(MarsMap, texture[6]); //Binding de la texture MarsMap
    tex.firstBindTexture(JupiterMap, texture[7]); //Binding de la texture JupiterMap
    tex.firstBindTexture(SaturneMap, texture[8]); //Binding de la texture SaturneMap
    tex.firstBindTexture(UranusMap, texture[9]); //Binding de la texture UranusMap
    tex.firstBindTexture(NeptuneMap, texture[10]); //Binding de la texture NeptuneMap
    tex.firstBindTexture(CallistoMap, texture[11]); //Binding de la texture CallistoMap
    /***************************/

    /* Sphere : planetes */
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sphere.getVertexCount() * sizeof(ShapeVertex), sphere.getDataPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, position)));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    /***************************/

    /* Tore : anneau de saturne */
    GLuint vbo_tore;
    glGenBuffers(1, &vbo_tore);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tore);
    glBufferData(GL_ARRAY_BUFFER, tore.getVertexCount() * sizeof(ShapeVertex), tore.getDataPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLuint vao_tore;
    glGenVertexArrays(1, &vao_tore);
    glBindVertexArray(vao_tore);
    glEnableVertexAttribArray(VERTEX_ATTR_POSITION);
    glEnableVertexAttribArray(VERTEX_ATTR_NORMAL);
    glEnableVertexAttribArray(VERTEX_ATTR_TEXCOORD);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_tore);
    glVertexAttribPointer(VERTEX_ATTR_POSITION, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, position)));
    glVertexAttribPointer(VERTEX_ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, normal)));
    glVertexAttribPointer(VERTEX_ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,  sizeof(ShapeVertex), (const GLvoid *)(offsetof(ShapeVertex, texCoords)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    /***************************/

    /* Transformations appliquer aux planetes */
    Transformation transfo;
    glm::vec3 rotateGlobal = glm::vec3(0, 1, 0);
    // Mercure
        // Translation
        sphere.setSphere(transfo.translate(sphere.getSphere(), 4));
        glm::vec3 translateMercure = transfo.applyTranslationX(sphere);
        // Scale
        sphere.setSphere(transfo.scale(sphere.getSphere(), 5));
        glm::vec3 scaleMercure = transfo.applyScale(sphere);
        // Rotation 
        sphere.setSphere(transfo.rotate(sphere.getSphere()));
        glm::vec3 rotateMercure = transfo.applyRotation(sphere);
    // Venus 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), 8));
        glm::vec3 translateVenus = transfo.applyTranslationX(sphere);
        // Scale 
        glm::vec3 scaleVenus = scaleMercure;
        // Rotation 
        glm::vec3 rotateVenus = rotateMercure;
    // Terre 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), -1.5));
        glm::vec3 translateEarth = transfo.applyTranslationX(sphere);
        // Scale 
        glm::vec3 scaleEarth = scaleMercure;
        // Rotation 
        glm::vec3 rotateEarth = rotateMercure;
        Sphere earthSphere = sphere;
    // Mars 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), -2));
        glm::vec3 translateMars = transfo.applyTranslationX(sphere);
        // Scale 
        glm::vec3 scaleMars = scaleMercure;
        // Rotation 
        glm::vec3 rotateMars = rotateMercure;
    // Jupiter 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), -3));
        glm::vec3 translateJupiter = transfo.applyTranslationX(sphere);
        // Scale 
        sphere.setSphere(transfo.scale(sphere.getSphere(), 0.3));
        glm::vec3 scaleJupiter = transfo.applyScale(sphere);
        // Rotation 
        glm::vec3 rotateJupiter = rotateMercure;
        Sphere jupiterSphere = sphere;
    // Saturne 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), -6));
        glm::vec3 translateSaturne = transfo.applyTranslationX(sphere);
        // Scale 
        sphere.setSphere(transfo.scale(sphere.getSphere(), 1.5));
        glm::vec3 scaleSaturne = transfo.applyScale(sphere);
        // Rotation 
        glm::vec3 rotateSaturne = rotateMercure;
    // Uranus 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), 1.5));
        glm::vec3 translateUranus = transfo.applyTranslationX(sphere);
        // Scale 
        glm::vec3 scaleUranus = scaleMercure;
        // Rotation 
        glm::vec3 rotateUranus = rotateMercure;
    // Neptune 
        // Translation 
        sphere.setSphere(transfo.translate(sphere.getSphere(), -2));
        glm::vec3 translateNeptune = transfo.applyTranslationX(sphere);
        // Scale 
        glm::vec3 scaleNeptune = scaleUranus;
        // Rotation 
        glm::vec3 rotateNeptune = rotateMercure;
    // Lune 
        // Translation 
        earthSphere.setSphere(transfo.translate(earthSphere.getSphere(), 7));
        glm::vec3 translateLune = transfo.applyTranslationX(earthSphere);
        // Scale 
        earthSphere.setSphere(transfo.scale(earthSphere.getSphere(), 1.2));
        glm::vec3 scaleLune = transfo.applyScale(earthSphere);
        // Rotation 
        glm::vec3 rotateLune = rotateMercure;
    // Callisto 
        // Translation 
        jupiterSphere.setSphere(transfo.translate(jupiterSphere.getSphere(), 5));
        glm::vec3 translateCallisto = transfo.applyTranslationX(jupiterSphere);
        // Scale 
        jupiterSphere.setSphere(transfo.scale(jupiterSphere.getSphere(), 5));
        glm::vec3 scaleCallisto = transfo.applyScale(jupiterSphere);
        // Rotation 
        glm::vec3 rotateCallisto = rotateMercure;
    /***************************/

    
    bool flag = false;
    bool done = false;
    FreeflyCamera Camera;
    float speedcam = 0.5;
    glm::ivec2 lastmousePos;
    float defaultspeed = 0.5;
    float boost_speed = 0.5;
    glm::mat4 ProjMatrix = glm::perspective(glm::radians(70.f), 800.f/600.f, 0.1f, 10000.f);
    // Application loop:
    while (!done) {
        // Event loop:
        SDL_Event e;
        while (windowManager.pollEvent(e)) {
            if (e.type == SDL_QUIT || windowManager.isKeyPressed(SDLK_ESCAPE)) {
                done = true; // Leave the loop after this iteration
            }
        }

        /* Gestion de la camera */
        if (windowManager.isMouseButtonPressed(SDL_BUTTON_LEFT) == true) {
            SDL_GetRelativeMouseState(&lastmousePos.x, &lastmousePos.y);
            if (flag == true) {
                Camera.rotateLeft(lastmousePos.x / 5);
                Camera.rotateUp(lastmousePos.y / 5);
            }
            flag = true;
        }
        else {
            flag = false;
        }
        // Ici on récupère les touches du clavier
        if (windowManager.isKeyPressed(SDLK_LCTRL) == true) { speedcam = defaultspeed * boost_speed; }
        else { speedcam = defaultspeed; }

        if (windowManager.isKeyPressed(SDLK_z) == true) { Camera.moveFront(speedcam); }
        if (windowManager.isKeyPressed(SDLK_q) == true) { Camera.moveLeft(speedcam); }

        if (windowManager.isKeyPressed(SDLK_s) == true) { Camera.moveFront(-speedcam); }

        if (windowManager.isKeyPressed(SDLK_d) == true) { Camera.moveLeft(-speedcam); }
        /***************************/

        /*********************************
         * HERE SHOULD COME THE RENDERING CODE
         *********************************/
        // Nettoyage de la fenêtre
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Affichage de la skybox
        glm::mat4  VMatrix = Camera.getViewMatrix();
        skybox.activeSkyBox(skytex, texSpatial, distRendu, ratio_h_w, VMatrix);

        glBindVertexArray(vao);
        glm::mat4 globalMVMatrix = Camera.getViewMatrix();

        // Soleil
        sunProgram.m_Program.use();
        glUniform1i(sunProgram.uTexture, 0);
        glm::mat4 sunMVMatrix = glm::rotate(globalMVMatrix, windowManager.getTime(), rotateGlobal);
        sunMVMatrix = glm::scale(sunMVMatrix, glm::vec3(5, 5, 5));
        glUniformMatrix4fv(sunProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(sunMVMatrix));
        glUniformMatrix4fv(sunProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(sunMVMatrix))));
        glUniformMatrix4fv(sunProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * sunMVMatrix));
        tex.activeAndBindTexture(GL_TEXTURE0, texture[0]);
        glDrawArrays(GL_TRIANGLES, 0, sphere.getVertexCount());
        glActiveTexture(GL_TEXTURE0);
        tex.activeAndBindTexture(GL_TEXTURE0, 0);
        glUniform1i(sunProgram.uTexture, 0);

        // Mercure
        drawPlanet(sphere, mercureProgram, tex, texture[4], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateMercure, scaleMercure, rotateMercure, 0.6);

        // Venus
        drawPlanet(sphere, venusProgram, tex, texture[5], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateVenus, scaleVenus, rotateVenus, 0.8);

        // Terre
        glm::mat4 earthMVMatrix = drawPlanet(sphere, earthProgram, tex, texture[3], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateEarth, scaleEarth, rotateEarth, 1);

        // Mars
        drawPlanet(sphere, marsProgram, tex, texture[6], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateMars, scaleMars, rotateMars, 1.2);

        // Jupiter
        glm::mat4 jupiterMVMatrix = drawPlanet(sphere, jupiterProgram, tex, texture[7], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateJupiter, scaleJupiter, rotateJupiter, 1.4);

        // Saturne
        drawPlanet(sphere, saturneProgram, tex, texture[8], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateSaturne, scaleSaturne, rotateSaturne, 0.5);

        // Uranus
        drawPlanet(sphere, uranusProgram, tex, texture[9], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateUranus, scaleUranus, rotateUranus, 1);

        // Neptune
        drawPlanet(sphere, neptuneProgram, tex, texture[10], windowManager,
            globalMVMatrix, ProjMatrix, rotateGlobal, translateNeptune, scaleNeptune, rotateNeptune, 1.5);

        // Lune autour de la Terre
        moonProgram.m_Program.use();
        glUniform1i(moonProgram.uTexture, 0);
        glm::mat4 moonMVMatrix = glm::rotate(earthMVMatrix, windowManager.getTime()*1, translateEarth);
        moonMVMatrix = glm::translate(moonMVMatrix, translateLune);
        moonMVMatrix = glm::scale(moonMVMatrix, scaleLune);
        moonMVMatrix = glm::rotate(moonMVMatrix, windowManager.getTime(), rotateLune);
        glUniformMatrix4fv(moonProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(moonMVMatrix));
        glUniformMatrix4fv(moonProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(moonMVMatrix))));
        glUniformMatrix4fv(moonProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * moonMVMatrix));
        tex.activeAndBindTexture(GL_TEXTURE0, texture[1]);
        glDrawArrays(GL_TRIANGLES, 0, earthSphere.getVertexCount());
        glActiveTexture(GL_TEXTURE0);
        tex.activeAndBindTexture(GL_TEXTURE0, 0);
        glUniform1i(moonProgram.uTexture, 0);

        // Callisto autour de Jupiter
        callistoProgram.m_Program.use();
        glUniform1i(callistoProgram.uTexture, 0);
        glm::mat4 callistoMVMatrix = glm::rotate(jupiterMVMatrix, windowManager.getTime()*1.4f, translateJupiter);
        callistoMVMatrix = glm::translate(callistoMVMatrix, translateCallisto);
        callistoMVMatrix = glm::scale(callistoMVMatrix, scaleCallisto);
        callistoMVMatrix = glm::rotate(callistoMVMatrix, windowManager.getTime(), rotateCallisto);
        glUniformMatrix4fv(callistoProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(callistoMVMatrix));
        glUniformMatrix4fv(callistoProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(callistoMVMatrix))));
        glUniformMatrix4fv(callistoProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * callistoMVMatrix));
        tex.activeAndBindTexture(GL_TEXTURE0, texture[11]);
        glDrawArrays(GL_TRIANGLES, 0, earthSphere.getVertexCount());
        glActiveTexture(GL_TEXTURE0);
        tex.activeAndBindTexture(GL_TEXTURE0, 0);
        glUniform1i(callistoProgram.uTexture, 0);

        // Tore : anneau de Saturne
        glBindVertexArray(vao_tore);
        saturneProgram.m_Program.use();
        glm::mat4 toreMVMatrix = glm::rotate(globalMVMatrix, windowManager.getTime() * 0.5f, glm::vec3(0, 1, 0)); // Translation * Rotation
        toreMVMatrix = glm::translate(toreMVMatrix, translateSaturne - glm::vec3(0, 2, -0.2));
        toreMVMatrix = glm::rotate(toreMVMatrix, 80.0f, glm::vec3(1,0,0));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture[1]);
        glUniform1i(saturneProgram.uTexture, 0);
        glUniformMatrix4fv(saturneProgram.uMVMatrix, 1, GL_FALSE, glm::value_ptr(toreMVMatrix));
        glUniformMatrix4fv(saturneProgram.uMVPMatrix, 1, GL_FALSE, glm::value_ptr(ProjMatrix * toreMVMatrix));
        glUniformMatrix4fv(saturneProgram.uNormalMatrix, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(toreMVMatrix))));
        glDrawArrays(GL_TRIANGLES, 0, tore.getVertexCount());
        glBindVertexArray(0);

        // Trajectoire de Mercure
        drawTore(TrajectoireMercure, vao_mercure, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateMercure);

        // Trajectoire de Venus
        drawTore(TrajectoireVenus, vao_venus, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateVenus);

        // Trajectoire de la Terre
        drawTore(TrajectoireTerre, vao_terre, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateEarth);

        // Trajectoire de Mars
        drawTore(TrajectoireMars, vao_mars, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateMars);

        // Trajectoire de Jupiter
        drawTore(TrajectoireJupiter, vao_jupiter, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateJupiter);

        // Trajectoire de Saturne
        drawTore(TrajectoireSaturne, vao_saturne, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateSaturne);

        // Trajectoire de Uranus
        drawTore(TrajectoireUranus, vao_uranus, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateUranus);

        // Trajectoire de Neptune
        drawTore(TrajectoireNeptune, vao_neptune, sunProgram, globalMVMatrix, windowManager,
            texture[1], ProjMatrix, translateNeptune);

        // Update the display
        windowManager.swapBuffers();
    }

    // Liberation de la memoire
    freeVboVao(vbo_mercure, vao_mercure);
    freeVboVao(vbo_venus, vao_venus);
    freeVboVao(vbo_terre, vao_terre);
    freeVboVao(vbo_mars, vao_mars);
    freeVboVao(vbo_jupiter, vao_jupiter);
    freeVboVao(vbo_saturne, vao_saturne);
    freeVboVao(vbo_uranus, vao_uranus);
    freeVboVao(vbo_neptune, vao_neptune);
    freeVboVao(vbo_tore, vao_tore);
    freeVboVao(vbo, vao);

    return EXIT_SUCCESS;
}
