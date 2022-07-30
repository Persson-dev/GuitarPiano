#include "GPGui.h"
#include "GPMusic.h"
#include "GPRenderer.h"
#include "GPData.h"
#include "GPSave.h"

#include "imgui.h"

#include <algorithm>
#include <memory>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

namespace gpgui {
namespace gui {

using Song = save::Song;
using ChordSave = save::ChordSave;

using ChordType = music::ChordType;
using ChordOffsets = music::ChordOffsets;
using Tab = music::Tab;
using Note = music::Note;

typedef std::shared_ptr<Song> SongPtr;

static bool pianoChordOnGuitar = true;

static int currentCapo = 0;

static const int CAPO_MIN = 0;
static const int CAPO_MAX = 10;

static ChordSave currentChord;

static int fretMax = 5;
static int currentOctave = 2;

static std::vector<SongPtr> loadedSongs;
static SongPtr editSong = nullptr;

constexpr ImVec4 SAVE_COLOR{ 0, 0.5, 0, 1 };
constexpr ImVec4 SAVE_HOVERED_COLOR{ 0, 0.7, 0, 1 };

constexpr ImVec4 DELETE_COLOR{ 0.5, 0, 0, 1 };
constexpr ImVec4 DELETE_HOVERED_COLOR{ 0.7, 0, 0, 1 };

static void ApplyPianoChord(ChordType ct, std::uint8_t note, int octave, const ChordOffsets& notes) {
	renderer::ClearKeyboard();
	for (int i = 0; i < notes.size(); i++) {
		if (notes[i] == data::EMPTY_NOTE)
			continue;
		renderer::SetKeyHighlight(notes[i], true);
	}
}

static void ApplyTab(const Tab& tab) {
	renderer::ClearTab();
	for (int i = 0; i < tab.size(); i++) {
		renderer::SetTab(i, tab[i]);
	}
}

static void ApplyTabPreview(const Tab& tab) {
	ApplyTab(tab);
	renderer::ClearKeyboard();
	for (int i = 0; i < tab.size(); i++) {
		if (tab[i] == data::EMPTY_TAB)
			continue;

		if (tab[i] == 0) {
			renderer::SetKeyHighlight(music::GetStringOffset(i) + currentCapo, true);
		} else {
			renderer::SetKeyHighlight(music::GetStringOffset(i) + tab[i], true);
		}
	}
}

static Tab GetTabChord(ChordType ct, std::uint8_t note, const ChordOffsets& notes) {
	if (pianoChordOnGuitar) {
		return music::FindPianoChord(notes, currentCapo, currentChord.fretMax);
	} else {
		return music::FindChord(music::GetChord(Note(note), ct), currentCapo);
	}
}

static void ApplyGuitarChord(ChordType ct, std::uint8_t note, int octave, const ChordOffsets& notes) {
	Tab tab = GetTabChord(ct, note, notes);
	ApplyTab(tab);
}

static void InverseChord(ChordOffsets& notes) {
	for (int i = 0; i < currentChord.inversion; i++) {
		std::uint8_t lastNote = notes[notes[3] == data::EMPTY_NOTE ? 2 : 3] - 12;
		std::uint8_t count = notes[3] == data::EMPTY_NOTE ? 2 : 3;
		for (int j = count; j > 0; j--) {
			notes[j] = notes[j - 1];
		}
		notes[0] = lastNote;
	}
}

static ChordOffsets OffsetsToNote(const ChordOffsets& offsets, int octave) {
	ChordOffsets notes;
	for (int i = 0; i < offsets.size(); i++) {
		if (offsets[i] == data::EMPTY_NOTE) {
			notes[i] = data::EMPTY_NOTE;
			continue;
		}
		notes[i] = currentChord.note + offsets[i] + 12 * octave;
	}
	return notes;
}

static void ApplyChord(ChordType ct, std::uint8_t note, int octave) {
	ChordOffsets offsets = GetChordOffsets(Note(note), ct);
	ChordOffsets notes = OffsetsToNote(offsets, octave);

	InverseChord(notes);
	ApplyPianoChord(ct, note, octave, notes);
	ApplyGuitarChord(ct, note, octave, notes);

	renderer::UpdateBuffers();
}

static void RefreshRendering() {
	if (currentChord.note != Note::TOTAL && currentChord.type != ChordType::COUNT)
		ApplyChord(currentChord.type, static_cast<std::uint8_t>(currentChord.note), currentChord.octave);
}

static void RenderChordButtons(ChordType ct) {
	for (int i = 0; i < Note::TOTAL; i++) {
		if (ImGui::Button(ToString(Note(i)).c_str())) {
			currentChord.note = Note(i);
			currentChord.type = ct;
			RefreshRendering();
		}
		ImGui::SameLine();
	}
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 100);
	if (ImGui::Button("Inversion")) {
		currentChord.inversion = (currentChord.inversion + 1) % 4;
		RefreshRendering();
	}
	ImGui::SameLine();
	ImGui::Text(": %i", currentChord.inversion);
	if (ImGui::SliderInt("Octave", &currentOctave, 1, 3) && currentChord.note != Note::TOTAL) {
		currentChord.octave = currentOctave;
		RefreshRendering();
	}
	if (ImGui::SliderInt("Fret max", &fretMax, 3, 12)) {
		currentChord.fretMax = fretMax;
		RefreshRendering();
	}
	if (currentChord.note != Note::TOTAL) {
		ImGui::Text("Accord actuel : %s %s", ToString(Note(currentChord.note)).c_str(), ToString(ct).c_str());
	}
}

static void RenderChordsTab() {
	if (ImGui::BeginTabItem("Accords")) {
		ImGui::BeginTabBar("Chords");
		for (int i = 0; i < static_cast<int>(ChordType::COUNT); i++) {
			ChordType ct = ChordType(i);
			if (ImGui::BeginTabItem(ToString(ct).c_str())) {
				ImGui::Text("%s :", ToString(ct).c_str());
				RenderChordButtons(ct);
				ImGui::EndTabItem();
			}
		}
		ImGui::EndTabBar();
		if (editSong != nullptr) {
			if (ImGui::Button("Ajouter l'accord")) {
				if (currentChord.note != Note::TOTAL) {
					editSong->chords.push_back(currentChord);
				}
			}
		}
		ImGui::EndTabItem();
	}
}

static void RenderOptions() {
	if (ImGui::BeginTabItem("Options")) {
		if (ImGui::Checkbox("Accords Guitaro-Piano ?", &pianoChordOnGuitar)) {
			currentChord.guitaroPiano = pianoChordOnGuitar;
			RefreshRendering();
		}
		if (editSong != nullptr) {
			ImGui::BeginDisabled();
		}
		if (ImGui::InputInt("Capo", &currentCapo, 1, 10)) {
			currentCapo = std::clamp(currentCapo, CAPO_MIN, CAPO_MAX);
			RefreshRendering();
			renderer::SetCapoPos(currentCapo);
		}
		if (editSong != nullptr) {
			ImGui::EndDisabled();
		}
		ImGui::EndTabItem();
	}
}

static void RenderSaveSongButton(const SongPtr& song) {
	ImGui::PushStyleColor(ImGuiCol_Button, SAVE_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SAVE_HOVERED_COLOR);
	if (ImGui::Button(std::string("Enregistrer##" + song->title).c_str())) {
		Song saveSong = *song;
		save::SaveSongToFile(saveSong, saveSong.title + ".gp");
	}
	ImGui::PopStyleColor(2);
}

static bool RenderDeleteSongButton(const SongPtr& song) {
	if (song == nullptr)
		return false;


	const std::string popupId = "DeleteSongConfirm" + song->title;

	bool deleted = false;

	ImGui::PushStyleColor(ImGuiCol_Button, DELETE_COLOR);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DELETE_HOVERED_COLOR);
	if (ImGui::Button(std::string("Supprimer##" + song->title).c_str())) {
		ImGui::OpenPopup(popupId.c_str());
	}
	if (ImGui::BeginPopup(popupId.c_str())) {
		ImGui::Text("Supprimer ?");
		if (ImGui::Button("Oui")) {
			ImGui::CloseCurrentPopup();
			deleted = true;
			std::string fileName = song->title + ".gp";
			if (fs::exists(fileName)) {  // removing file if it exists
				fs::remove(fileName);
			}
			auto it = std::find(loadedSongs.begin(), loadedSongs.end(), song);
			loadedSongs.erase(it);
		}
		ImGui::PopStyleColor(2);
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, SAVE_COLOR);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SAVE_HOVERED_COLOR);
		if (ImGui::Button("Non")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor(2);
		ImGui::EndPopup();
	} else {
		ImGui::PopStyleColor(2);
	}
	return deleted;
}

static void RenderSongs() {
	if (loadedSongs.empty()) {
		ImGui::Text("Aucune chanson chargée");
		return;
	}
	ImGui::BeginChild("Songs");
	for (auto& song : loadedSongs) {
		ImGui::Text("%s (capo %i)", song->title.c_str(), song->capo);
		ImGui::SameLine();
		if (song == editSong) {
			ImGui::BeginDisabled();
			ImGui::Button("Séléctionnée");
			ImGui::EndDisabled();
		} else {
			if (ImGui::Button(std::string("Sélectionner##" + song->title).c_str())) {
				editSong = song;
				currentCapo = song->capo;
				renderer::SetCapoPos(currentCapo);
				RefreshRendering();
			}
		}
		ImGui::SameLine();
		RenderSaveSongButton(song);
		ImGui::SameLine();
		RenderDeleteSongButton(song);
	}
	ImGui::EndChild();
}

static void RenderNewSongPopup() {
	if (ImGui::BeginPopup("##New Song Popup")) {
		static char buffer[512];
		static int songCapo = 0;
		ImGui::InputText("Nom de la chanson", buffer, sizeof(buffer));
		ImGui::InputInt("Capo", &songCapo, 1);
		songCapo = std::clamp(songCapo, CAPO_MIN, CAPO_MAX);
		if (ImGui::Button("Créer une nouvelle chanson")) {
			loadedSongs.push_back(std::make_shared<Song>(buffer, static_cast<save::CapoPosType>(songCapo)));
			// resetting buffers
			songCapo = 0;
			std::fill(std::begin(buffer), std::end(buffer), 0);
			// closing popup
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

static void AddSongsInDirectory() {
	for (const auto& entry : fs::directory_iterator(".")) {
		auto path = entry.path();
		if (path.extension().string() == ".gp") {
			std::string fileName = path.filename().string();
			Song newSong = save::LoadSongFromFile(fileName);
			if (newSong.title.empty())
				continue;

			auto it = std::find_if(loadedSongs.begin(), loadedSongs.end(), [&newSong](SongPtr song) {
				return newSong.title == song->title;
			});

			if (it == loadedSongs.end()) { // add only if does not already exist
				loadedSongs.push_back(std::make_shared<Song>(newSong));
			}
		}
	}
}

static void RenderSongsTab() {
	if (ImGui::BeginTabItem("Chansons")) {
		if (ImGui::Button("Nouveau")) {
			ImGui::OpenPopup("##New Song Popup");
		}
		RenderNewSongPopup();
		ImGui::SameLine();
		if (ImGui::Button("Actualiser")) {
			AddSongsInDirectory();
		}
		ImGui::Separator();
		RenderSongs();
		ImGui::EndTabItem();
	}
}

unsigned int HashTab(const Tab& tab) {
	return std::hash<unsigned int>()(tab[0] & 0xF << 20 | tab[1] & 0xF << 16
		| tab[2] & 0xF << 12 | tab[3] & 0xF << 8 | tab[4] & 0xF << 4
		| tab[5] & 0xF);
}

static void RenderSongFrames() {
	static int eraseIndex = 0;
	for (int i = 0; i < editSong->chords.size(); i++) {
		ImGui::BeginChildFrame(100 + i * 100, ImVec2(200, 190));
		ImGui::Text(music::ToString(editSong->chords[i]).c_str());
		if (i > 0) {
			if (ImGui::Button("<-")) {
				auto previousTab = editSong->chords[i - 1];
				editSong->chords[i - 1] = editSong->chords[i];
				editSong->chords[i] = previousTab;
			}
			ImGui::SameLine();
		}
		if (i < editSong->chords.size() - 1) {
			if (ImGui::Button("->")) {
				auto nextTab = editSong->chords[i + 1];
				editSong->chords[i + 1] = editSong->chords[i];
				editSong->chords[i] = nextTab;
			}
		} else {
			ImGui::NewLine();
		}
		if (ImGui::Button("Visualiser")) {
			currentChord = editSong->chords[i];
			currentOctave = currentChord.octave;
			pianoChordOnGuitar = currentChord.guitaroPiano;
			RefreshRendering();
		}
		if (ImGui::Button("Supprimer")) {
			ImGui::OpenPopup("EraseTab");
		}
		if (ImGui::BeginPopup("EraseTab")) {
			ImGui::Text("Supprimer ?");
			if (ImGui::Button("Oui")) {
				editSong->chords.erase(editSong->chords.begin() + i);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Non")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		ImGui::EndChildFrame();
		ImGui::SameLine();
	}
}

static void RenderEditTab() {
	if (ImGui::BeginTabItem("Edition")) {
		if (editSong == nullptr) {
			ImGui::Text("Sélectionnez une chanson pour commencer");
		} else {
			ImGui::Text("Edition de %s (capo %i)", editSong->title.c_str(), editSong->capo);

			ImGui::BeginChild("SongContent", {}, false, ImGuiWindowFlags_HorizontalScrollbar);
			RenderSongFrames();
			ImGui::NewLine();
			RenderSaveSongButton(editSong);
			ImGui::SameLine();
			if (RenderDeleteSongButton(editSong)) {
				editSong = nullptr;
			}
			ImGui::SameLine();
			if (ImGui::Button("Terminé")) {
				editSong = nullptr;
			}
			ImGui::SameLine();
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
}

static void RenderInfos() {
	if (ImGui::BeginTabItem("Infos")) {
		ImGui::Text("FPS : %i", (int) std::ceil(ImGui::GetIO().Framerate));
	}
}

static void RenderTabs() {
	ImGui::BeginTabBar("MainTab");
	RenderChordsTab();
	RenderEditTab();
	RenderSongsTab();
	RenderOptions();
	RenderInfos();
	ImGui::EndTabBar();
}

void Render() {
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Begin("Piano", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos({ 0, 0 }, ImGuiCond_Always);
	ImGui::SetWindowSize({ io.DisplaySize.x, io.DisplaySize.y * 0.4f }, ImGuiCond_Always);
	RenderTabs();
	ImGui::End();
}

void Init() {
	AddSongsInDirectory();
	currentChord.octave = currentOctave;  // adjust the slider
	currentChord.fretMax = fretMax;
	currentChord.guitaroPiano = pianoChordOnGuitar;
}

} // namespace gui
} // namespace gpgui
