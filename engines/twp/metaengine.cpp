/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/action.h"
#include "common/translation.h"
#include "common/savefile.h"
#include "engines/advancedDetector.h"
#include "graphics/scaler.h"
#include "gui/gui-manager.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"
#include "gui/ThemeEval.h"
#include "image/png.h"
#include "twp/twp.h"
#include "twp/detection.h"
#include "twp/metaengine.h"
#include "twp/detection.h"
#include "twp/savegame.h"
#include "twp/time.h"
#include "twp/actions.h"
#include "twp/debugtools.h"
#include "twp/dialogs.h"

#define MAX_SAVES 99

const char *TwpMetaEngine::getName() const {
	return "twp";
}

Common::Error TwpMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Twp::TwpEngine(syst, (const Twp::TwpGameDescription*)(desc));
	return Common::kNoError;
}

bool TwpMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		   (f == kSupportsLoadingDuringStartup);
}

int TwpMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

void TwpMetaEngine::registerDefaultSettings(const Common::String &) const {
	ConfMan.registerDefault("toiletPaperOver", false);
	ConfMan.registerDefault("annoyingInJokes", false);
	ConfMan.registerDefault("invertVerbHighlight", true);
	ConfMan.registerDefault("retroFonts", false);
	ConfMan.registerDefault("retroVerbs", false);
	ConfMan.registerDefault("hudSentence", false);
	ConfMan.registerDefault("ransomeUnbeeped", false);
	ConfMan.registerDefault("language", "en");
}

static Common::String getDesc(const Twp::SaveGame &savegame) {
	Common::String desc = Twp::formatTime(savegame.time, "%b %d at %H:%M");
	if (savegame.easyMode)
		desc += " (casual)";
	return desc;
}

SaveStateDescriptor TwpMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(filename);

	if (f) {

		Common::InSaveFile *thumbnailFile = g_system->getSavefileManager()->openForLoading(filename + ".png");

		// Create the return descriptor
		SaveStateDescriptor desc(this, slot, "?");

		if (thumbnailFile) {
			Image::PNGDecoder png;
			if (png.loadStream(*thumbnailFile)) {
				Graphics::ManagedSurface *thumbnail = new Graphics::ManagedSurface();
				thumbnail->copyFrom(*png.getSurface());
				Graphics::Surface *thumbnailSmall = new Graphics::Surface();
				createThumbnail(thumbnailSmall, thumbnail);
				desc.setThumbnail(thumbnailSmall);
			}
		}
		Twp::SaveGame savegame;
		Twp::SaveGameManager::getSaveGame(f, savegame);
		Common::String savegameDesc(getDesc(savegame));
		Twp::DateTime dt = Twp::toDateTime(savegame.time);
		desc.setDescription(savegameDesc);
		desc.setPlayTime(savegame.gameTime * 1000);
		desc.setSaveDate(dt.year, dt.month, dt.day);
		desc.setSaveTime(dt.hour, dt.min);

		return desc;
	}

	return SaveStateDescriptor();
}

GUI::OptionsContainerWidget *TwpMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	GUI::OptionsContainerWidget *widget = new Twp::TwpOptionsContainerWidget(boss, name, target);
	return widget;
}

Common::Array<Common::Keymap *> TwpMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *engineKeyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, target, "Thimbleweed Park keymap");

	struct {
		const char *name;
		const char *desc;
		Twp::TwpAction action;
		const char *input;
	} actions[] = {
		{"SKIPCUTSCENE", _s("Skip cutscene"), Twp::kSkipCutscene, "ESCAPE"},
		{"SELECTACTOR1", _s("Select Actor 1"), Twp::kSelectActor1, "1"},
		{"SELECTACTOR2", _s("Select Actor 2"), Twp::kSelectActor2, "2"},
		{"SELECTACTOR3", _s("Select Actor 3"), Twp::kSelectActor3, "3"},
		{"SELECTACTOR4", _s("Select Actor 4"), Twp::kSelectActor4, "4"},
		{"SELECTACTOR5", _s("Select Actor 5"), Twp::kSelectActor5, "5"},
		{"SELECTACTOR6", _s("Select Actor 6"), Twp::kSelectActor6, "6"},
		{"SELECTCHOICE1", _s("Select Choice 1"), Twp::kSelectChoice1, "1"},
		{"SELECTCHOICE2", _s("Select Choice 2"), Twp::kSelectChoice2, "2"},
		{"SELECTCHOICE3", _s("Select Choice 3"), Twp::kSelectChoice3, "3"},
		{"SELECTCHOICE4", _s("Select Choice 4"), Twp::kSelectChoice4, "4"},
		{"SELECTCHOICE5", _s("Select Choice 5"), Twp::kSelectChoice5, "5"},
		{"SELECTCHOICE6", _s("Select Choice 6"), Twp::kSelectChoice6, "6"},
		{"SELECTNEXTACTOR", _s("Select Next Actor"), Twp::kSelectNextActor, "0"},
		{"SELECTPREVACTOR", _s("Select Previous Actor"), Twp::kSelectPreviousActor, "9"},
		{"SKIPTEXT", _s("Skip Text"), Twp::kSkipText, "."},
		{"SHOWHOTSPOTS", _s("Show hotspots"), Twp::kShowHotspots, "TAB"},
		{0, 0, Twp::kSkipCutscene, 0},
	};

	Common::Action *act;
	for (int i = 0; actions[i].name; i++) {
		act = new Common::Action(actions[i].name, _(actions[i].desc));
		act->setCustomEngineActionEvent(actions[i].action);
		act->addDefaultInputMapping(actions[i].input);
		engineKeyMap->addAction(act);
	}

	return Common::Keymap::arrayOf(engineKeyMap);
}

#if PLUGIN_ENABLED_DYNAMIC(TWP)
REGISTER_PLUGIN_DYNAMIC(TWP, PLUGIN_TYPE_ENGINE, TwpMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TWP, PLUGIN_TYPE_ENGINE, TwpMetaEngine);
#endif
