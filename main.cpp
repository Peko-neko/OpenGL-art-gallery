#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Vertex and index data
float vertices[] = {
    // Positions               // Colors         // Texture Coords
    -3.0f,  1.0f, -10.0f,      1.0f, 0.0f, 0.0f,  0.0f, 5.0f, // Top-left back
     3.0f,  1.0f, -10.0f,      0.0f, 1.0f, 0.0f,  1.0f, 5.0f, // Top-right back
     3.0f, -1.0f, -10.0f,      0.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Bottom-right back
    -3.0f, -1.0f, -10.0f,      1.0f, 1.0f, 0.0f,  0.0f, 0.0f, // Bottom-left back
    -3.0f,  1.0f,   10.0f,     1.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Top-left front
     3.0f,  1.0f,   10.0f,     0.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Top-right front
     3.0f, -1.0f,   10.0f,     1.0f, 1.0f, 1.0f,  1.0f, 5.0f, // Bottom-right front
    -3.0f, -1.0f,   10.0f,     0.5f, 0.5f, 0.5f,  0.0f, 5.0f  // Bottom-left front
};

unsigned int indices[] = {
    // Back wall
    0, 1, 2, 0, 2, 3,
    // Left wall
    0, 4, 7, 0, 7, 3,
    // Right wall
    1, 5, 6, 1, 6, 2,
    // Bottom wall (floor)
    3, 2, 6, 3, 6, 7,
    // Top wall (ceiling)
    0, 1, 5, 0, 5, 4
};

// GLFW callback for window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

unsigned int loadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Art Gallery", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int wallTexture = loadTexture("white-wall-textures.jpg");
    unsigned int floorTexture = loadTexture("wood-floor-textures.jpg");
    unsigned int ceilingTexture = loadTexture("ceiling.jpg");

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);

        // Bind and draw walls
        glBindTexture(GL_TEXTURE_2D, wallTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int))); // Back wall
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int))); // Left wall
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int))); // Right wall

        // Bind and draw floor
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(unsigned int))); // Floor

        // Bind and draw ceiling
        glBindTexture(GL_TEXTURE_2D, ceilingTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(24 * sizeof(unsigned int))); // Ceiling

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &wallTexture);
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &ceilingTexture);

    glfwTerminate();
    return 0;
}
