#pragma once

#include "GPMusic.h"

namespace gpgui {
namespace save {

typedef std::uint8_t CapoPosType;

struct ChordSave {
	music::Note note : 4;
	music::ChordType type : 3;
	bool guitaroPiano : 1;
	std::uint8_t octave : 2;
	std::uint8_t inversion : 2;
	std::uint8_t fretMax : 4;

	ChordSave() : note(music::Note::TOTAL) {}
};

struct Song {
	std::string title;
	CapoPosType capo;
	std::vector<ChordSave> chords;

	Song(const std::string& songTitle, CapoPosType songCapo) : title(songTitle), capo(songCapo) {}
};

void SaveSongToFile(const Song& save, const std::string& fileName);
Song LoadSongFromFile(const std::string& filePath);

} // namespace save
} // namespace gpgui
