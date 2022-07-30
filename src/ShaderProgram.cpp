#include "ShaderProgram.h"

#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>

namespace gpgui {

ShaderProgram::ShaderProgram() :
	m_ProgramID(0), m_VertexShaderID(0), m_FragmentShaderID(0) {
}

ShaderProgram::~ShaderProgram() {
	CleanUp();
}

void ShaderProgram::Start() const {
	glUseProgram(m_ProgramID);
}

void ShaderProgram::Stop() const {
	glUseProgram(0);
}

void ShaderProgram::CleanUp() const {
	Stop();
	glDetachShader(m_ProgramID, m_VertexShaderID);
	glDetachShader(m_ProgramID, m_FragmentShaderID);
	glDeleteShader(m_VertexShaderID);
	glDeleteShader(m_FragmentShaderID);
	glDeleteProgram(m_ProgramID);
}

void ShaderProgram::LoadProgramFile(const std::string& vertexFile,
	const std::string& fragmentFile) {
	m_VertexShaderID = static_cast<unsigned int>(LoadShaderFromFile(vertexFile, static_cast<unsigned int>(GL_VERTEX_SHADER)));
	m_FragmentShaderID = static_cast<unsigned int>(LoadShaderFromFile(fragmentFile, static_cast<unsigned int>(GL_FRAGMENT_SHADER)));
	m_ProgramID = glCreateProgram();
	glAttachShader(m_ProgramID, m_VertexShaderID);
	glAttachShader(m_ProgramID, m_FragmentShaderID);
	glLinkProgram(m_ProgramID);
	glValidateProgram(m_ProgramID);
	GetAllUniformLocation();
}

void ShaderProgram::LoadProgram(const std::string& vertexSource,
	const std::string& fragmentSource) {
	m_VertexShaderID = static_cast<unsigned int>(LoadShader(vertexSource, static_cast<unsigned int>(GL_VERTEX_SHADER)));
	m_FragmentShaderID = static_cast<unsigned int>(LoadShader(fragmentSource, static_cast<unsigned int>(GL_FRAGMENT_SHADER)));
	m_ProgramID = glCreateProgram();
	glAttachShader(m_ProgramID, m_VertexShaderID);
	glAttachShader(m_ProgramID, m_FragmentShaderID);
	glLinkProgram(m_ProgramID);
	glValidateProgram(m_ProgramID);
	GetAllUniformLocation();
}

unsigned int ShaderProgram::LoadShader(const std::string& source, unsigned int type) {
	unsigned int shaderID = glCreateShader(GLenum(type));

	const char* c_str = source.c_str();
	int* null = 0;
	glShaderSource(shaderID, 1, &c_str, null);
	glCompileShader(shaderID);
	GLint compilesuccessful;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compilesuccessful);
	if (compilesuccessful == false) {
		GLsizei size;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &size);
		std::vector<char> shaderError(static_cast<std::size_t>(size));
		glGetShaderInfoLog(shaderID, size, &size, shaderError.data());
	}
	return shaderID;
}

unsigned int ShaderProgram::LoadShaderFromFile(const std::string& file, unsigned int type) {
	std::stringstream stream;
	std::ifstream fileStream(file);

	if (fileStream) {
		stream << fileStream.rdbuf();
	} else {
		return 0;
	}

	return LoadShader(stream.str(), type);
}

} // namespace gpgui
