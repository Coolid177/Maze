//opengl includes
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <irrKlang/irrKlang.h>

#include <ft2build.h>
#include FT_FREETYPE_H

//other includes
#include <iostream>
#include <map>

#include "shader.h"
#include "MazeHandler.h"
#include "CollisionDetector.h"
#include "InteractionDetector.h"

#include "Model.h"
#include "Mesh.h"

#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window, CollisionDetector* detector, InteractionDetector* interactionDetector);
unsigned int loadCubemap(vector<std::string> faces);
unsigned int loadTexture(char const* path);
void generateMaze(int width, int height);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
glm::vec3 cameraPos = glm::vec3(16.0f, 7.0f, -10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraMoveFront = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 defaultCameraSize = glm::vec3(0.5f, 2.0f, 0.5f);
glm::vec3 cameraSize = defaultCameraSize;

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//jump data
float ySpeed = 0.0f;
const float acceleration = 8 * (-9.81f);

void processY(float deltaTime, CollisionDetector* detector);
void processYSpeed(float deltaTime);

//crouch data
bool crouched = false;
bool canTransitionCrouch = true;

//interaction data
bool canInteract = false;
bool interacted = false;
float reach = 3.0f;
std::vector<int> collectedIDs;

bool checkCollectedObjects(int ID);

//start flag
bool hasMoved = false;

//text data
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

std::map<char, Character> Characters;

void renderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int& VAO, unsigned int& VBO, bool centerAlignment);
int prepareTextRendering(Shader& shader, unsigned int& VAO, unsigned int& VBO);

//flashlight setting
bool flashOn = false;

//umph sound engine
irrklang::ISoundEngine* umphSoundEngine = irrklang::createIrrKlangDevice();
irrklang::ISoundSource* umphSound = umphSoundEngine->addSoundSourceFromFile("Sound/umph.mp3");

glm::mat4 lukasModel = glm::mat4(1.0f);

int main(int argc, char* argv[]) {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Horror game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // load maze
    // --------
    generateMaze(50, 50);
    MazeHandler maze = MazeHandler("maze.txt");
    vector<glm::vec3> positions = maze.getBuildingPositionos();
    // positions of lighting elements
    vector<glm::vec3> pointLightPositions = maze.getLightPositions();
    vector<glm::vec3> trashPositions = maze.getTrashPositions();
    cameraPos = maze.spawnLocation();

    // plane vertices
    float planeVertices[] = {
        //vertex             //normals             //texture
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
    };

    // cube vertices
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // cubes
    // -----    
    unsigned int VBO, VAO;
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // light VAO
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // plane
    // -----
    unsigned int PLANEVBO, PLANEVAO;

    glGenVertexArrays(1, &PLANEVAO);
    glGenBuffers(1, &PLANEVBO);

    glBindVertexArray(PLANEVAO);

    glBindBuffer(GL_ARRAY_BUFFER, PLANEVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals coord attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // skybox
    // ------
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces
    {
        "Skybox/Spacebox/right.png",
        "Skybox/Spacebox/left.png",
        "Skybox/Spacebox/top.png",
        "Skybox/Spacebox/bottom.png",
        "Skybox/Spacebox/front.png",
        "Skybox/Spacebox/back.png"
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    // load and create a texture 
    // -------------------------
    unsigned int texture = loadTexture("floor.jpg");

    // configure shaders
    // -----------------
    Shader skyboxShader("Skybox/skybox.vs", "Skybox/skybox.fs");
    Shader lightingShader("light_object.vs", "light_object.fs");
    Shader textShader("text.vs", "text.fs");
    Shader modelShader("model_loading.vs", "model_loading.fs");

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //text rendering
    unsigned int textVAO, textVBO;
    if (prepareTextRendering(textShader, textVAO, textVBO) == -1) {
        return -1;
    }

    // create collision detector object
    // --------------------------------
    CollisionDetector detector = CollisionDetector();
    InteractionDetector interactionDetector = InteractionDetector(reach);

    // create mesh 
    // -----------
    Model building("Meshes/City meshes/building.obj");
    Model spaceship("Meshes/Ufo/UFO.obj");
    Model trash("Meshes/Trash Pile/Garbage Bag obj.obj");

    // instance meshes
    // ---------------
    // building
    glm::mat4* buildingModelMatrices = new glm::mat4[positions.size()];
    for (int i = 0; i < positions.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, positions.at(i));
        buildingModelMatrices[i] = model;
    }

    // spaceship
    glm::mat4* spaceShipModelMatrices = new glm::mat4[pointLightPositions.size()];
    for (int i = 0; i < pointLightPositions.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPositions.at(i));
        model = glm::scale(model, glm::vec3(4.0f));
        spaceShipModelMatrices[i] = model;
    }

    // trash
    glm::mat4* trashModelMatrices = new glm::mat4[trashPositions.size()];
    for (int i = 0; i < trashPositions.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, trashPositions.at(i));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        trashModelMatrices[i] = model;
    }

    // play background sound
    // ---------------------
    irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();
    SoundEngine->play2D("Sound/background sound.mp3", true);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > float(1.0f / 15.0f)) {
            deltaTime = float(1.0f / 15.0f);
        }
        
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // process input
        // -------------
        processInput(window, &detector, &interactionDetector);

        if (hasMoved) {
            processYSpeed(deltaTime);
            processY(deltaTime, &detector);
        }

        detector.clearMazeObjects();
        interactionDetector.clearMazeObjects();

        // render maze
        // ----------
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100000.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 model = glm::mat4(1.0f);

        // render lights
        // -------------
        lightingShader.use();
        lightingShader.setVec3("viewPos", cameraPos);
        lightingShader.setFloat("material.shininess", 90.0f);

        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        lightingShader.setVec3("dirLight.diffuse", 0.05f, 0.05f, 0.05f);
        // point light 1
        for (int i = 0; i < 30; i++) {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions.at(i));
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.9f,2.0f, 1.0f);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.0f, 2.0f, 0.0f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
        }
        // spotLight
        lightingShader.setVec3("spotLight.position", cameraPos);
        lightingShader.setVec3("spotLight.direction", cameraFront);
        if (flashOn) {
            lightingShader.setVec3("spotLight.ambient", 1.0f, 1.5f, 1.0f);
            lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        }
        else {
            lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            lightingShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);

        }
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // draw light sources
        // ------------------
        modelShader.use();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);
        for (unsigned int i = 0; i < 30; i++) {
            modelShader.setMat4("model", spaceShipModelMatrices[i]);
            spaceship.Draw(modelShader);
        }

        // render buildings
        // ----------------
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        for (unsigned int i = 0; i < positions.size(); i++) {
            MazeObject* mazeObject = new MazeObject(positions.at(i), glm::vec3(20, 25.0f, 15.0f));
            //draw instanced mesh
            lightingShader.setMat4("model", buildingModelMatrices[i]);
            building.Draw(lightingShader);
            //add mesh to collision detector
            detector.addMazeObject(mazeObject);
        }

        // render interaction objects
        // --------------------------
        lightingShader.use();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);
        for (unsigned int i = 0; i < trashPositions.size(); i++) {
            if (!checkCollectedObjects(i)) {
                InteractionObject* interactionObject = new InteractionObject(trashPositions[i], glm::vec3(8.0f, 14.0f, 8.0f), i);
                interactionDetector.addInteractionObject(*interactionObject);
                detector.addMazeObject(interactionObject);
                lightingShader.setMat4("model", trashModelMatrices[i]);
                building.Draw(lightingShader);
            }
        }

        // render plane
        // ------------
        lightingShader.use();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        glBindVertexArray(PLANEVAO);
        glBindBuffer(GL_ARRAY_BUFFER, PLANEVBO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        model = glm::mat4(1.0f);
        glm::vec3 position = glm::vec3(maze.getMazeWidth() / 1.4, -1.5f, -maze.getMazeHeight() / 3.1);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec3 size = glm::vec3(maze.getMazeWidth() * 100, maze.getMazeHeight() * 100, 1.0);
        model = glm::scale(model, size);
        lightingShader.setMat4("model", model);
        size = glm::vec3(size.y, size.z * 10, size.x);
        MazeObject *mazeObject = new MazeObject(position, size);
        detector.addMazeObject(mazeObject);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // draw skybox as last
        // -------------------
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp))); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        //render text
        if (hasMoved != true) {
            renderText(textShader, "movement: z, q, s, d  utility: g for flashlight, f for interaction", (float)SCR_WIDTH / 2 -60.0f, (float)SCR_HEIGHT / 2 + 15.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f), textVAO, textVBO, true);
            renderText(textShader, "move to start", (float)SCR_WIDTH / 2, (float)SCR_HEIGHT / 2 - 15.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f), textVAO, textVBO, true);
        }
        else if (canInteract) {
            renderText(textShader, "Press 'F' to interact", (float)SCR_WIDTH / 2, (float)SCR_HEIGHT / 2, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f), textVAO, textVBO, true);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!hasMoved) {
        return;
    }
    
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw));
    cameraMoveFront = glm::normalize(front);
    front.x *= cos(glm::radians(pitch));
    front.z *= cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window, CollisionDetector* detector, InteractionDetector* interactionDetector){
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(30 * deltaTime);
    if (crouched) {
        cameraSpeed *= 0.5;
    }
    glm::vec3 newPos = cameraPos;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        hasMoved = true;
        newPos += (cameraSpeed / 2) * (cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f) - glm::normalize(glm::cross(cameraMoveFront, cameraUp)));
    }
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        hasMoved = true;
        newPos += (cameraSpeed / 2) * (cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f) + glm::normalize(glm::cross(cameraMoveFront, cameraUp)));
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        hasMoved = true;
        newPos -= (cameraSpeed / 2) * (cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f) + glm::normalize(glm::cross(cameraMoveFront, cameraUp)));
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        hasMoved = true;
        newPos -= (cameraSpeed / 2) * (cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f) - glm::normalize(glm::cross(cameraMoveFront, cameraUp)));
    }
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        hasMoved = true;
        newPos += cameraSpeed * cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f);
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        hasMoved = true;
        newPos -= cameraSpeed * cameraMoveFront * glm::vec3(1.0f, 0.0f, 1.0f);
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        hasMoved = true;
        newPos -= glm::normalize(glm::cross(cameraMoveFront, cameraUp)) * cameraSpeed;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        hasMoved = true;
        newPos += glm::normalize(glm::cross(cameraMoveFront, cameraUp)) * cameraSpeed;
    }
    if (newPos != cameraPos && !detector->checkCameraCollisions(newPos, cameraSize)) {
        cameraPos = newPos;
    } 
    //play umph sound on collision
    if (detector->checkCameraCollisions(newPos, cameraSize) && !umphSoundEngine->isCurrentlyPlaying(umphSound)) {
        umphSoundEngine->play2D(umphSound, false);
    }
    if (canTransitionCrouch && (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)) {
        hasMoved = true;
        if (crouched) {
            cameraSize = defaultCameraSize;
            cameraPos += defaultCameraSize * glm::vec3(0.0f, 0.5f, 0.0f);
        }
        else {
            cameraSize = defaultCameraSize * glm::vec3(1.0f, 0.5f, 1.0f);
            cameraPos -= defaultCameraSize * glm::vec3(0.0f, 0.5f, 0.0f);
        }
        crouched = !crouched;
        canTransitionCrouch = false;
    }
    else if (!canTransitionCrouch && (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_RELEASE)) {
        canTransitionCrouch = true;
    }
    if (!crouched && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && ySpeed == 0.0f) {
        hasMoved = true;
        ySpeed = 30.0f;
        processY(deltaTime, detector);
    }
    if (interactionDetector->checkInteractions(cameraPos, cameraFront)) {
        canInteract = true;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            interacted = true;
            collectedIDs.push_back(interactionDetector->getInteractedID());
        }
    }
    else {
        canInteract = false;
    }
    //flashlight toggle
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        hasMoved = true;
        flashOn = !flashOn;
    }
}

void processY(float deltaTime, CollisionDetector* detector) {
    float currentY = (cameraPos * glm::vec3(0.0, 1.0, 0.0)).y;
    currentY = currentY + ySpeed * deltaTime + 0.5 * acceleration * deltaTime * deltaTime;
    glm::vec3 newPos = cameraPos * glm::vec3(1.0, 0.0, 1.0) + currentY * glm::vec3(0.0, 1.0, 0.0);
    if (!detector->checkCameraCollisions(newPos, cameraSize)) {
        cameraPos = newPos;
    }
    else {
        ySpeed = 0.0f;
    }
}

void processYSpeed(float deltaTime) {
    ySpeed = ySpeed + acceleration * deltaTime;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

int prepareTextRendering(Shader &shader,unsigned int &textVAO, unsigned int &textVBO) {
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // find path to font
    std::string font_name = "fonts/arial.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);
        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return 0;
}

pair<float, float> calculateCenterIndentation(std::string text, bool x, bool y, float scale) {
    pair<float, float> indent = pair<float, float>(0.0f,0.0f);

    // iterate through all characters
    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        if (x) {
            indent.first += ch.Size.x * scale;
        }
        if (y && indent.second < ch.Size.y * scale) {
            indent.second = ch.Size.y * scale;
        }
    }

    indent.first /= 2;
    indent.second /= 2;

    return indent;
}

void renderText(Shader& s, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int& VAO, unsigned int& VBO, bool centerAlignment) {
    // activate corresponding render state	
    s.use();
    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    pair<float, float> indent;
    if (centerAlignment) {
        indent = calculateCenterIndentation(text, true, true, scale);
    }
    else {
        indent = calculateCenterIndentation(text, false, true, scale);
    }

    // iterate through all characters
    std::string::const_iterator c;

    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale - indent.first;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale - indent.second;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

bool checkCollectedObjects(int ID) {
    for (int i = 0; i < collectedIDs.size(); i++)
    {
        if (ID == collectedIDs[i]) {
            return true;
        }
    }
    return false;
}