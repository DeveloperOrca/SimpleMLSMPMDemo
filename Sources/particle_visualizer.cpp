#include "particle_visualizer.h"

namespace internal{
    void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
    {
        int size = std::min(width, height);
        glViewport(0, 0, size, size);
    }
}

bool particle_visualizer::initialize() {
    // set running time
    m_running_time_begin = SClock::now();
    std::cout << log_prefix(log_type::INFO) << "Initialize Visualizer\n";
    // initialize GLFW
    if(!glfwInit()) {
        const char* description;
        glfwGetError(&description);
        std::cout << log_prefix(log_type::ERROR) << "Failed initialize GLFW\n"
                  << description << "\n";
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Initialize GLFW\n";

    // open window
    constexpr int VERSION_MAJOR = 4;
    constexpr int VERSION_MINOR = 2;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr int WIDTH = 500;
    constexpr int HEIGHT = 500;
    constexpr char const* TITLE = "MPMDemo";

    m_window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
    if(m_window == nullptr) {
        const char* description;
        glfwGetError(&description);

        std::cout << log_prefix(log_type::ERROR) << "Failed to create window\n"
                  << description << "\n";
        glfwTerminate();
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Initialize GLFW window successfully\n";

    // make context
    glfwMakeContextCurrent(m_window);

    // set call back
    glfwSetFramebufferSizeCallback(m_window, internal::framebuffer_resize_callback);

    // initialize GLAD
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << log_prefix(log_type::ERROR) << "Failed to initialize GLAD\n";
        glfwTerminate();
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Initialize GLAD successfully\n";

    // set view port
    glViewport(0, 0, WIDTH, HEIGHT);

    // show openGL prop
    std::cout << log_prefix(log_type::INFO) << "Current GL info\n"
              << "\tversion  : " << glGetString(GL_VERSION) << "\n"
              << "\trenderer : " << glGetString(GL_RENDERER) << "\n";

    std::cout << log_prefix(log_type::INFO) << "Initialized Visualizer successfully\n\n";
    return true;
}

bool particle_visualizer::load_shader() {
    std::cout << log_prefix(log_type::INFO) << "Load shaders\n";

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* vert_src_c_str =
        #include "ShaderSrc/particle_visualizer.vert"
        ;
    // try compile
    glShaderSource(vertex_shader, 1, &vert_src_c_str, nullptr);
    glCompileShader(vertex_shader);

    int is_success;
    char info_log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_success);
    if (!is_success)
    {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cout << log_prefix(log_type::ERROR) << "Failed to compile the shader\n"
                  << "\t" << info_log << "\n";
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Compiled vertex shader\n";

    const char* frag_src_c_str =
        #include "ShaderSrc/particle_visualizer.frag"
        ;
    // try compile
    glShaderSource(fragment_shader, 1, &frag_src_c_str, nullptr);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_success);
    if (!is_success)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cout << log_prefix(log_type::ERROR) << "Failed to compile the shader\n"
                  << "\t" << info_log << "\n";
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Compiled fragment shader\n";

    // try to link shader
    m_shader_program = glCreateProgram();
    glAttachShader(m_shader_program, vertex_shader);
    glAttachShader(m_shader_program, fragment_shader);
    glLinkProgram(m_shader_program);
    // check for linking errors
    glGetProgramiv(m_shader_program, GL_LINK_STATUS, &is_success);
    if (!is_success) {
        glGetProgramInfoLog(m_shader_program, 512, nullptr, info_log);
        std::cout << log_prefix(log_type::ERROR) << "Failed to link shaders\n"
                  << "\t" << info_log << "\n";
        return false;
    }
    std::cout << log_prefix(log_type::INFO) << "Linked shaders\n";

    // clean up
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    std::cout << log_prefix(log_type::INFO) << "Load shaders successfully\n\n";
    return true;
}

std::string particle_visualizer::log_prefix(particle_visualizer::log_type _log_type) {
    static std::array<const char*, 3> log_type_str = {"ERROR", "WARN", "INFO"};
    std::chrono::duration<double> time_span = SClock::now() - m_running_time_begin;

    char buf[30];
    sprintf(buf, "[%12.2fs|%5s]", time_span.count(), log_type_str[static_cast<int>(_log_type)]);

    return buf;
}

void particle_visualizer::init_buffer() {
    glGenBuffers(2, m_buffers);
    glGenVertexArrays(1, &m_vertex_array);

    glBindVertexArray(m_vertex_array);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[0]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[1]);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    std::cout << log_prefix(log_type::INFO) << "Initialized buffers and vertex array successfully\n\n";
}

bool particle_visualizer::should_close() {
    return glfwWindowShouldClose(m_window);
}


