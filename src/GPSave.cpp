#include "GPSave.h"

#include <cstring>
#include <fstream>
#include <sstream>

namespace gpgui {
namespace save {

static constexpr std::uint8_t SAVE_VERSION = 0;

typedef std::vector<std::uint8_t> DataBuffer;
typedef std::uint16_t SongSizeType;

template<typename T>
static void WriteData(DataBuffer& buffer, const T* data, std::size_t dataSize = sizeof(T)) {
	std::size_t endPos = buffer.size();

	buffer.resize(endPos + dataSize);
	std::memcpy(buffer.data() + endPos, data, dataSize);
}

static void WriteFile(const DataBuffer& buffer, const std::string& fileName) {
	std::ofstream fileStream(fileName);

	if (!fileStream)
		return;

	fileStream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

void SaveSongToFile(const Song& song, const std::string& fileName) {
	DataBuffer buffer;

	WriteData(buffer, &SAVE_VERSION);  // writing file version

	WriteData(buffer, &song.capo);  // writing the capo offset as 8 bit unsigned integer

	SongSizeType songSize = song.chords.size();
	WriteData(buffer, &songSize);  // writing the size as 16 bit unsigned int

	WriteData(buffer, song.chords.data(), songSize * sizeof(ChordSave));

	WriteFile(buffer, fileName);
}

static Song LoadSongVersion0(const std::string& data, std::size_t offset, const std::string& filePath) {
	Song song{ "", 0 };

	std::memcpy(&song.capo, data.data() + offset, sizeof(song.capo));  // reading capo pos
	offset += sizeof(song.capo);

	SongSizeType songSize;
	std::memcpy(&songSize, data.data() + offset, sizeof(songSize));  // reading song size

	offset += sizeof(songSize);

	song.chords.resize(songSize);
	std::memcpy(song.chords.data(), data.data() + offset, songSize * sizeof(ChordSave));  // reading chords

	song.title = filePath.substr(0, filePath.find_last_of('.'));

	return song;
}

Song LoadSongFromFile(const std::string& filePath) {
	std::ifstream fileStream(filePath);

	if (!fileStream)
		return { "", 0 };

	std::ostringstream oss;
	oss << fileStream.rdbuf();

	std::string str = oss.str();

	std::uint8_t fileVersion;
	std::memcpy(&fileVersion, str.data(), sizeof(fileVersion));  // reading file save version

	std::size_t offset = sizeof(fileVersion);

	switch (fileVersion) {
	case 0:
		return LoadSongVersion0(str, offset, filePath);

	default:
		return { "", 0 };
	}
}

} // namespace save
} // namespace gpgui
