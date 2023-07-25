#include <glad/gl.h> 
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>

#include "camera.h"
#include "scene.h"
#include "renderer.h"


Camera camera;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int width = 800;
int height = 600;
float lastX = (float)width / 2.0f;
float lastY = (float)height / 2.0f;

bool firstMouse = true;
bool ui_interaction = false;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // TODO: Fix the non-default framebuffer to also be resized
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // If ui interaction mode do not move camera
    if (ui_interaction)
        return;

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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        ui_interaction = !ui_interaction;
        if (ui_interaction) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Renderer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    // Successfully loaded OpenGL
    printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // Viewport and callbacks
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    // Scene generates cube and plane in constructor atm
    Scene scene(&camera);

    // Renderer to specify forward/deferred rendering
    Renderer renderer(width, height, &camera);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Input
        processInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool screen_blur = false;
        static bool visualize_normals = false;
        //static float gamma = 2.2f;
        static float exposure = 1.0f;
        static glm::mat3 temp_kernel(
            0, 0, 0,
            0, 1, 0,
            0, 0, 0
        );

        ImGui::Begin("Render options");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        if (ImGui::Checkbox("Visualize Normals", &visualize_normals)) {
            scene.setVisualizeNormals(visualize_normals);
        }
        //ImGui::SliderFloat("Gamma Correction", &gamma, 0.0f, 5.0f);
        if (ImGui::SliderFloat("HDR Exposure", &exposure, 0.0f, 10.0f)) {
            renderer.setExposure(exposure);
        }

        static int current_render_type = RenderType::DEFERRED;
        const char* render_type_names[] = { "Deferred", "Forward", "Debug Cube Depthmap"};
        const char* current_render_type_name = (current_render_type >= 0 && current_render_type < RenderType::COUNT) ? render_type_names[current_render_type] : "Unknown";
        ImGui::SliderInt("Render Type", &current_render_type, 0, RenderType::COUNT - 1, current_render_type_name);

        ImGui::Text("Kernel applied in post-processing");
        ImGui::InputFloat3("R1", &temp_kernel[0][0]);
        ImGui::InputFloat3("R2", &temp_kernel[1][0]);
        ImGui::InputFloat3("R3", &temp_kernel[2][0]);
        if (ImGui::Button("Apply Kernel")) {
            renderer.setKernel(temp_kernel);
        }
        ImGui::End();

        // Render scene with current render type
        renderer.render(scene, static_cast<RenderType>(current_render_type));

        // Render Imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // terminate imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}
