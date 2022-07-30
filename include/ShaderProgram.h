#pragma once

#include <string>

namespace gpgui {

class ShaderProgram {
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	void Start() const;
	void Stop() const;

	void LoadProgramFile(const std::string& vertexFile, const std::string& fragmentFile);
	void LoadProgram(const std::string& vertexSource, const std::string& fragmentSource);

protected:
	virtual void GetAllUniformLocation() = 0;

	void CleanUp() const;

private:
	unsigned int m_ProgramID;
	unsigned int m_VertexShaderID;
	unsigned int m_FragmentShaderID;

	unsigned int LoadShaderFromFile(const std::string& file, unsigned int type);
	unsigned int LoadShader(const std::string& source, unsigned type);
};

} // namespace gpgui
