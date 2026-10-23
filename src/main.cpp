#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "SupraLayer.hpp"
using namespace geode::prelude;

class $modify(SupraMenu, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;
		auto spr = ButtonSprite::create("Supra");
		auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SupraMenu::onSupra));
		btn->setID("supramundo-button"_spr);
		if (auto menu = this->getChildByID("bottom-menu")) {
			menu->addChild(btn);
			menu->updateLayout();
		}
		return true;
	}
	void onSupra(CCObject*) {
		CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, SupraLayer::scene()));
	}
};
