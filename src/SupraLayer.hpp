#pragma once
#include <Geode/Geode.hpp>
#include <set>
#include <vector>
#include <string>
using namespace geode::prelude;

enum class Biome { Pradera, Nieve, Pantano, Desierto, Lava };
enum class Kind { Level, Tower, Shop, Chest, King, Monster };
enum class Pending { None, PlayLevel, UnlockGate, BuyItem };

struct BiomeInfo {
	const char* name;
	CCRect rect;
	ccColor4F color;
	std::vector<std::string> keys; // palabras clave para encajar niveles
};

struct MapNode {
	std::string id;
	Kind kind;
	Biome biome;
	CCPoint pos;
	std::string label;
	CCNode* view = nullptr;
	CCLabelBMFont* text = nullptr;
	GJGameLevel* level = nullptr;
};

struct Gate {
	std::string id;
	CCRect rect;
	int cost;
	CCNode* view = nullptr;
};

const std::vector<BiomeInfo>& biomes();

class SupraLayer : public CCLayer, public LevelManagerDelegate, public FLAlertLayerProtocol {
public:
	static SupraLayer* create();
	static CCScene* scene();

protected:
	CCSize m_win;
	CCNode* m_world = nullptr;
	CCDrawNode* m_player = nullptr;
	CCLabelBMFont* m_hudPts = nullptr;
	CCLabelBMFont* m_hudBag = nullptr;
	CCLabelBMFont* m_hudBiome = nullptr;
	CCPoint m_pos = ccp(300, 300);
	CCPoint m_target;
	bool m_hasTarget = false;
	int m_keys = 0; // bits: 1 arriba 2 abajo 4 izq 8 der
	float m_gateCooldown = 0.f;
	int m_stage = 0;
	std::vector<MapNode> m_nodes;
	std::vector<Gate> m_gates;
	std::set<std::string> m_unlocked;
	std::vector<GJGameLevel*> m_recent, m_featured;
	std::string m_pendingNode;
	Pending m_pending = Pending::None;
	std::string m_pendingId;
	int m_pendingItem = 0;

	bool init() override;
	void onEnter() override;
	void onExit() override;
	void update(float dt) override;
	void keyBackClicked() override;
	void registerWithTouchDispatcher() override;
	bool ccTouchBegan(CCTouch*, CCEvent*) override;
	void keyDown(enumKeyCodes key);
	void keyDown(enumKeyCodes key, double);
	void keyUp(enumKeyCodes key);
	void keyUp(enumKeyCodes key, double);
	void handleKey(enumKeyCodes key, bool down);

	void buildTerrain();
	void buildNodes();
	void buildGates();
	void buildPlayer();
	void buildHud();
	void refreshHud();

	bool walkable(CCPoint p);
	Gate* lockedGateAt(CCPoint p);
	void tryMove(CCPoint p);
	void arrive();
	void interact(MapNode& n);
	void promptGate(Gate& g);
	void promptShop(MapNode& n);
	void openChest(MapNode& n);
	void talkKing();
	void poke(MapNode& n);
	void floatText(CCPoint p, std::string const& s, ccColor3B col = {255, 255, 120});
	void checkReward();
	int levelsDone();
	int towersDone();
	MapNode* findNode(std::string const& id);

	void requestLevels();
	void assignLevels();
	void loadLevelsFinished(CCArray* levels, const char* key);
	void loadLevelsFinished(CCArray* levels, const char* key, int);
	void loadLevelsFailed(const char* key);
	void loadLevelsFailed(const char* key, int);
	void onBack(CCObject*);
	void FLAlert_Clicked(FLAlertLayer* alert, bool btn2) override;
};
