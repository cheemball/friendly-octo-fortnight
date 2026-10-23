#include "SupraLayer.hpp"
#include <random>
#include <algorithm>
#include <cmath>

// ---------- Datos ----------
const std::vector<BiomeInfo>& biomes() {
	static const std::vector<BiomeInfo> b = {
		{"Pradera", CCRect(0, 0, 1400, 1200), {0.35f, 0.75f, 0.30f, 1}, {"spring", "primavera", "flower", "forest", "grass", "garden", "green", "nature", "tree", "bosque"}},
		{"Zona de Nieve", CCRect(1400, 0, 1400, 1200), {0.85f, 0.93f, 1.0f, 1}, {"snow", "ice", "winter", "frozen", "nieve", "hielo", "cold", "christmas", "navidad", "deadlocked"}},
		{"Pantano", CCRect(0, 1200, 1400, 1200), {0.25f, 0.33f, 0.20f, 1}, {"swamp", "toxic", "poison", "marsh", "pantano", "dark", "spooky", "slime", "ghost", "zombie"}},
		{"Desierto y Piramides", CCRect(1400, 1200, 1400, 1200), {0.93f, 0.80f, 0.45f, 1}, {"desert", "egypt", "pyramid", "sand", "desierto", "ship", "portal", "pharaoh", "temple", "sun"}},
		{"Inframundo", CCRect(2800, 600, 1400, 1200), {0.45f, 0.12f, 0.08f, 1}, {"lava", "fire", "volcano", "hell", "demon", "fuego", "magma", "inferno", "flame", "burn"}},
	};
	return b;
}

static const float WORLD_W = 4200.f, WORLD_H = 2400.f;
static std::mt19937 rng(1337);
static float rf(float a, float b) { return a + (b - a) * (float)(rng() % 10000) / 10000.f; }

static int geti(char const* k) { return Mod::get()->getSavedValue<int>(k, 0); }
static void seti(char const* k, int v) { Mod::get()->setSavedValue<int>(k, v); }
static int geti(std::string const& k) { return Mod::get()->getSavedValue<int>(k, 0); }
static void seti(std::string const& k, int v) { Mod::get()->setSavedValue<int>(k, v); }

static const char* KING_LINES[] = {
	"Ah... un viajero. Soy The King. Estas tierras no siempre estuvieron rotas.",
	"Cinco fragmentos de poder sostenian este mundo: pradera, nieve, pantano, arena y fuego.",
	"El pantano guarda una torre. Quien la domine oira el primer eco de mi corona.",
	"Hace mucho, mi reino era un solo mapa. Un demonio lo partio en biomas.",
	"Las torres no son prisiones: son cerraduras. Y tu eres la llave.",
	"Los baguetes no son simple moneda. Eran el pan del pueblo. Guardalos bien.",
	"Bajo el volcan duerme el ultimo jefe. Lleva mi rostro... y mi culpa.",
	"Has llegado lejos. Cuando caiga la ultima torre, sabras quien soy de verdad.",
};
static const int KING_N = 8;

struct ShopItem { char const* name; char const* desc; int cost; };
static const ShopItem SHOP[] = {
	{"Mapa del Tesoro", "Revela todos los cofres ocultos.", 20},
	{"Llave Maestra", "Abre una puerta bloqueada gratis.", 35},
	{"Botas Veloces", "Aumenta tu velocidad en el mapa.", 50},
};
static const int SHOP_N = 3;

// ---------- Decoracion ----------
static void tree(CCDrawNode* d, CCPoint p, ccColor4F leaf) {
	CCPoint t[4] = {ccp(p.x - 3, p.y - 14), ccp(p.x + 3, p.y - 14), ccp(p.x + 3, p.y), ccp(p.x - 3, p.y)};
	d->drawPolygon(t, 4, {0.4f, 0.25f, 0.1f, 1}, 0, {0, 0, 0, 0});
	d->drawDot(ccp(p.x, p.y + 10), 14, leaf);
}
static void tri(CCDrawNode* d, CCPoint p, float w, float h, ccColor4F c) {
	CCPoint t[3] = {ccp(p.x - w, p.y), ccp(p.x + w, p.y), ccp(p.x, p.y + h)};
	d->drawPolygon(t, 3, c, 0, {0, 0, 0, 0});
}

static void decorate(CCDrawNode* d, int bi) {
	auto& b = biomes()[bi];
	auto r = b.rect;
	for (int i = 0; i < 55; i++) {
		CCPoint p = ccp(rf(r.origin.x + 40, r.origin.x + r.size.width - 40), rf(r.origin.y + 40, r.origin.y + r.size.height - 40));
		switch (bi) {
			case 0: tree(d, p, {0.15f, 0.5f + rf(0, 0.2f), 0.15f, 1}); break;
			case 1: tri(d, p, 12, 30, {0.6f, 0.8f, 0.9f, 1}); d->drawDot(ccp(p.x, p.y + 30), 4, {1, 1, 1, 1}); break;
			case 2: d->drawDot(p, rf(8, 18), {0.15f, 0.25f + rf(0, 0.1f), 0.2f, 1}); d->drawSegment(p, ccp(p.x, p.y + 25), 1.5f, {0.3f, 0.4f, 0.15f, 1}); break;
			case 3:
				if (i % 9 == 0) tri(d, p, 45, 60, {0.85f, 0.7f, 0.3f, 1});
				else d->drawSegment(p, ccp(p.x, p.y + 22), 3, {0.2f, 0.5f, 0.2f, 1});
				break;
			case 4: d->drawDot(p, rf(8, 20), {0.25f, 0.2f, 0.2f, 1}); if (i % 6 == 0) tri(d, p, 14, 35, {1.f, 0.4f, 0.1f, 1}); break;
		}
	}
	if (bi == 4) { // rios de lava
		for (int k = 0; k < 3; k++) {
			float base = r.origin.y + 250 + k * 380;
			CCPoint prev = ccp(r.origin.x + 20, base);
			for (float x = r.origin.x + 60; x < r.origin.x + r.size.width - 20; x += 40) {
				CCPoint cur = ccp(x, base + sinf(x * 0.012f + k) * 60);
				d->drawSegment(prev, cur, 18, {1.f, 0.35f, 0.05f, 1});
				d->drawSegment(prev, cur, 8, {1.f, 0.85f, 0.2f, 1});
				prev = cur;
			}
		}
	}
	if (bi == 3) { // barco
		CCPoint hull[4] = {ccp(r.origin.x + 200, r.origin.y + 200), ccp(r.origin.x + 320, r.origin.y + 200), ccp(r.origin.x + 300, r.origin.y + 160), ccp(r.origin.x + 220, r.origin.y + 160)};
		d->drawPolygon(hull, 4, {0.45f, 0.28f, 0.12f, 1}, 0, {0, 0, 0, 0});
		d->drawSegment(ccp(r.origin.x + 260, r.origin.y + 200), ccp(r.origin.x + 260, r.origin.y + 260), 2, {0.3f, 0.2f, 0.1f, 1});
		tri(d, ccp(r.origin.x + 260, r.origin.y + 205), 30, 50, {1, 1, 1, 1});
	}
}

// ---------- Creacion ----------
SupraLayer* SupraLayer::create() {
	auto ret = new SupraLayer();
	if (ret && ret->init()) { ret->autorelease(); return ret; }
	CC_SAFE_DELETE(ret);
	return nullptr;
}
CCScene* SupraLayer::scene() {
	auto s = CCScene::create();
	s->addChild(SupraLayer::create());
	return s;
}

bool SupraLayer::init() {
	if (!CCLayer::init()) return false;
	m_win = CCDirector::get()->getWinSize();
	m_world = CCNode::create();
	this->addChild(m_world);
	// Fondo
	auto bg = CCLayerColor::create({20, 24, 30, 255});
	this->addChild(bg, -10);

	for (auto& g : std::vector<std::string>{"g1", "g2", "g3", "g4", "g5"})
		if (Mod::get()->getSavedValue<bool>("gate_" + g, false)) m_unlocked.insert(g);

	buildTerrain();
	buildGates();
	buildNodes();
	buildPlayer();
	buildHud();

	this->setTouchEnabled(true);
	this->setKeyboardEnabled(true);
	this->setKeypadEnabled(true);
	this->scheduleUpdate();
	requestLevels();
	return true;
}

void SupraLayer::buildTerrain() {
	auto d = CCDrawNode::create();
	m_world->addChild(d, 0);
	auto& bs = biomes();
	for (int i = 0; i < (int)bs.size(); i++) {
		auto r = bs[i].rect;
		CCPoint v[4] = {ccp(r.origin.x + 10, r.origin.y + 10), ccp(r.origin.x + r.size.width - 10, r.origin.y + 10),
			ccp(r.origin.x + r.size.width - 10, r.origin.y + r.size.height - 10), ccp(r.origin.x + 10, r.origin.y + r.size.height - 10)};
		d->drawPolygon(v, 4, bs[i].color, 4, {0, 0, 0, 1});
		decorate(d, i);
		auto name = CCLabelBMFont::create(bs[i].name, "bigFont.fnt");
		name->setScale(1.2f);
		name->setOpacity(130);
		name->setPosition(ccp(r.origin.x + r.size.width / 2, r.origin.y + r.size.height - 70));
		m_world->addChild(name, 1);
	}
}

void SupraLayer::buildGates() {
	m_gates = {
		{"g1", CCRect(1370, 520, 60, 160), 30},
		{"g2", CCRect(620, 1170, 160, 60), 80},
		{"g3", CCRect(2020, 1170, 160, 60), 150},
		{"g4", CCRect(1320, 1770, 160, 60), 200},
		{"g5", CCRect(2770, 820, 60, 160), 300},
	};
	for (auto& g : m_gates) {
		auto n = CCNode::create();
		auto d = CCDrawNode::create();
		auto r = g.rect;
		CCPoint v[4] = {ccp(r.origin.x, r.origin.y), ccp(r.origin.x + r.size.width, r.origin.y), ccp(r.origin.x + r.size.width, r.origin.y + r.size.height), ccp(r.origin.x, r.origin.y + r.size.height)};
		d->drawPolygon(v, 4, {0.5f, 0.1f, 0.6f, 0.9f}, 3, {1, 1, 1, 1});
		n->addChild(d);
		auto l = CCLabelBMFont::create(fmt::format("{} pts", g.cost).c_str(), "goldFont.fnt");
		l->setScale(0.5f);
		l->setPosition(ccp(r.getMidX(), r.getMidY()));
		n->addChild(l);
		m_world->addChild(n, 2);
		g.view = n;
		if (m_unlocked.count(g.id)) n->setVisible(false);
	}
}

void SupraLayer::buildNodes() {
	auto add = [&](std::string id, Kind k, Biome b, float x, float y, std::string label) {
		MapNode n; n.id = id; n.kind = k; n.biome = b; n.pos = ccp(x, y); n.label = label;
		m_nodes.push_back(n);
	};
	// Pradera
	add("p1", Kind::Level, Biome::Pradera, 500, 500, "Nivel"); add("p2", Kind::Level, Biome::Pradera, 900, 350, "Nivel"); add("p3", Kind::Level, Biome::Pradera, 1100, 850, "Nivel");
	add("shop1", Kind::Shop, Biome::Pradera, 250, 800, "Tienda"); add("king", Kind::King, Biome::Pradera, 700, 950, "The King");
	add("m1", Kind::Monster, Biome::Pradera, 650, 300, ""); add("m2", Kind::Monster, Biome::Pradera, 1000, 600, "");
	add("c1", Kind::Chest, Biome::Pradera, 150, 1050, "");
	// Nieve
	add("n1", Kind::Level, Biome::Nieve, 1700, 300, "Nivel"); add("n2", Kind::Level, Biome::Nieve, 2100, 700, "Nivel"); add("n3", Kind::Level, Biome::Nieve, 2500, 400, "Nivel");
	add("m3", Kind::Monster, Biome::Nieve, 1900, 950, ""); add("m4", Kind::Monster, Biome::Nieve, 2400, 900, "");
	add("c2", Kind::Chest, Biome::Nieve, 2650, 150, "");
	// Pantano
	add("s1", Kind::Level, Biome::Pantano, 300, 1500, "Nivel"); add("s2", Kind::Level, Biome::Pantano, 700, 1900, "Nivel"); add("s3", Kind::Level, Biome::Pantano, 1100, 1500, "Nivel");
	add("t1", Kind::Tower, Biome::Pantano, 1200, 2200, "Torre"); add("m5", Kind::Monster, Biome::Pantano, 400, 2200, ""); add("m6", Kind::Monster, Biome::Pantano, 900, 1700, "");
	add("c3", Kind::Chest, Biome::Pantano, 100, 2300, "");
	// Desierto
	add("d1", Kind::Level, Biome::Desierto, 1700, 1500, "Nivel"); add("d2", Kind::Level, Biome::Desierto, 2100, 1800, "Nivel"); add("d3", Kind::Level, Biome::Desierto, 2500, 1500, "Nivel");
	add("t2", Kind::Tower, Biome::Desierto, 2300, 2250, "Torre"); add("shop2", Kind::Shop, Biome::Desierto, 1600, 2200, "Tienda"); add("m7", Kind::Monster, Biome::Desierto, 2000, 1400, "");
	add("c4", Kind::Chest, Biome::Desierto, 2700, 2300, "");
	// Lava
	add("l1", Kind::Level, Biome::Lava, 3100, 900, "Nivel"); add("l2", Kind::Level, Biome::Lava, 3500, 1200, "Nivel"); add("l3", Kind::Level, Biome::Lava, 3900, 900, "Nivel");
	add("t3", Kind::Tower, Biome::Lava, 3500, 1600, "Torre Final"); add("m8", Kind::Monster, Biome::Lava, 3200, 1500, ""); add("m9", Kind::Monster, Biome::Lava, 3800, 1500, "");
	add("c5", Kind::Chest, Biome::Lava, 4100, 700, "");

	// caminos
	auto paths = CCDrawNode::create();
	m_world->addChild(paths, 1);
	for (size_t i = 1; i < m_nodes.size(); i++) {
		auto &a = m_nodes[i - 1], &b = m_nodes[i];
		if (a.biome == b.biome && a.kind != Kind::Chest && b.kind != Kind::Chest && a.kind != Kind::Monster && b.kind != Kind::Monster)
			paths->drawSegment(a.pos, b.pos, 6, {0.55f, 0.42f, 0.25f, 0.8f});
	}

	bool names = Mod::get()->getSettingValue<bool>("mostrar-nombres");
	for (auto& n : m_nodes) {
		auto root = CCNode::create();
		root->setPosition(n.pos);
		auto d = CCDrawNode::create();
		switch (n.kind) {
			case Kind::Level: d->drawDot(ccp(0, 0), 20, {1, 0.9f, 0.2f, 1}); d->drawDot(ccp(0, 0), 12, {0.9f, 0.5f, 0.1f, 1}); break;
			case Kind::Tower: {
				CCPoint v[4] = {ccp(-22, 0), ccp(22, 0), ccp(22, 70), ccp(-22, 70)};
				d->drawPolygon(v, 4, {0.2f, 0.1f, 0.25f, 1}, 3, {0.9f, 0.1f, 0.1f, 1});
				tri(d, ccp(0, 70), 28, 28, {0.8f, 0.1f, 0.1f, 1});
				break;
			}
			case Kind::Shop: { CCPoint v[4] = {ccp(-26, 0), ccp(26, 0), ccp(26, 30), ccp(-26, 30)}; d->drawPolygon(v, 4, {0.9f, 0.7f, 0.3f, 1}, 2, {0.3f, 0.2f, 0.1f, 1}); tri(d, ccp(0, 30), 32, 20, {0.8f, 0.2f, 0.2f, 1}); break; }
			case Kind::Chest: { CCPoint v[4] = {ccp(-14, 0), ccp(14, 0), ccp(14, 18), ccp(-14, 18)}; d->drawPolygon(v, 4, {0.6f, 0.35f, 0.1f, 1}, 2, {1, 0.85f, 0.2f, 1}); break; }
			case Kind::King: d->drawDot(ccp(0, 14), 16, {0.95f, 0.8f, 0.6f, 1}); { CCPoint c[3] = {ccp(-14, 26), ccp(14, 26), ccp(0, 44)}; d->drawPolygon(c, 3, {1, 0.85f, 0.1f, 1}, 0, {0, 0, 0, 0}); } break;
			case Kind::Monster: d->drawDot(ccp(0, 0), 14, {0.7f, 0.2f, 0.6f, 1}); d->drawDot(ccp(-5, 3), 3, {1, 1, 1, 1}); d->drawDot(ccp(5, 3), 3, {1, 1, 1, 1}); break;
		}
		root->addChild(d);
		if (!n.label.empty() && n.kind != Kind::Chest && n.kind != Kind::Monster) {
			auto l = CCLabelBMFont::create(n.label.c_str(), "bigFont.fnt");
			l->setScale(0.35f);
			l->setPosition(ccp(0, (n.kind == Kind::Tower ? 105 : 40)));
			l->setVisible(names || n.kind == Kind::King || n.kind == Kind::Shop);
			root->addChild(l);
			n.text = l;
		}
		m_world->addChild(root, 3);
		n.view = root;
		if (n.kind == Kind::Chest) root->setVisible(false);
	}
}

void SupraLayer::buildPlayer() {
	m_player = CCDrawNode::create();
	m_player->drawDot(ccp(0, 0), 13, {0.1f, 0.9f, 0.3f, 1});
	m_player->drawDot(ccp(0, 0), 8, {0.1f, 0.4f, 1.0f, 1});
	m_world->addChild(m_player, 10);
	m_player->setPosition(m_pos);
}

void SupraLayer::buildHud() {
	auto menu = CCMenu::create();
	auto back = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(SupraLayer::onBack));
	menu->addChild(back);
	menu->setPosition(ccp(30, m_win.height - 30));
	this->addChild(menu, 50);
	m_hudPts = CCLabelBMFont::create("", "goldFont.fnt");
	m_hudBag = CCLabelBMFont::create("", "goldFont.fnt");
	m_hudBiome = CCLabelBMFont::create("", "bigFont.fnt");
	for (auto l : {m_hudPts, m_hudBag}) { l->setScale(0.5f); l->setAnchorPoint(ccp(1, 1)); }
	m_hudPts->setPosition(ccp(m_win.width - 10, m_win.height - 8));
	m_hudBag->setPosition(ccp(m_win.width - 10, m_win.height - 28));
	m_hudBiome->setScale(0.45f);
	m_hudBiome->setPosition(ccp(m_win.width / 2, m_win.height - 18));
	this->addChild(m_hudPts, 50); this->addChild(m_hudBag, 50); this->addChild(m_hudBiome, 50);
	refreshHud();
}

void SupraLayer::refreshHud() {
	m_hudPts->setString(fmt::format("Puntos: {}", geti("puntos")).c_str());
	m_hudBag->setString(fmt::format("Baguetes: {}  Llaves: {}", geti("baguetes"), geti("llaves")).c_str());
}

// ---------- Movimiento ----------
bool SupraLayer::walkable(CCPoint p) {
	for (auto& b : biomes()) {
		auto r = b.rect;
		CCRect in(r.origin.x + 10, r.origin.y + 10, r.size.width - 20, r.size.height - 20);
		if (in.containsPoint(p)) return true;
	}
	for (auto& g : m_gates)
		if (m_unlocked.count(g.id) && g.rect.containsPoint(p)) return true;
	return false;
}

Gate* SupraLayer::lockedGateAt(CCPoint p) {
	for (auto& g : m_gates) {
		if (m_unlocked.count(g.id)) continue;
		CCRect r(g.rect.origin.x - 14, g.rect.origin.y - 14, g.rect.size.width + 28, g.rect.size.height + 28);
		if (r.containsPoint(p)) return &g;
	}
	return nullptr;
}

void SupraLayer::tryMove(CCPoint p) {
	if (walkable(p)) { m_pos = p; return; }
	if (auto g = lockedGateAt(p)) {
		m_hasTarget = false;
		if (m_gateCooldown <= 0.f) { m_gateCooldown = 3.f; promptGate(*g); }
		return;
	}
	if (walkable(ccp(p.x, m_pos.y))) m_pos.x = p.x;
	else if (walkable(ccp(m_pos.x, p.y))) m_pos.y = p.y;
	else m_hasTarget = false;
}

void SupraLayer::update(float dt) {
	if (m_gateCooldown > 0.f) m_gateCooldown -= dt;
	float speed = (float)Mod::get()->getSettingValue<double>("velocidad") * (geti("botas") ? 1.4f : 1.f);
	float step = speed * dt;
	CCPoint dir = ccp(0, 0);
	if (m_keys & 1) dir.y += 1; if (m_keys & 2) dir.y -= 1;
	if (m_keys & 4) dir.x -= 1; if (m_keys & 8) dir.x += 1;
	if (dir.x != 0 || dir.y != 0) {
		m_hasTarget = false;
		m_pendingNode.clear();
		float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
		tryMove(ccp(m_pos.x + dir.x / len * step, m_pos.y + dir.y / len * step));
	} else if (m_hasTarget) {
		CCPoint d = ccp(m_target.x - m_pos.x, m_target.y - m_pos.y);
		float len = sqrtf(d.x * d.x + d.y * d.y);
		if (len <= step) { tryMove(m_target); m_hasTarget = false; arrive(); }
		else tryMove(ccp(m_pos.x + d.x / len * step, m_pos.y + d.y / len * step));
	}
	m_player->setPosition(m_pos);

	CCPoint cam = ccp(m_win.width / 2 - m_pos.x, m_win.height / 2 - m_pos.y);
	cam.x = std::max(m_win.width - WORLD_W, std::min(0.f, cam.x));
	cam.y = std::max(m_win.height - WORLD_H, std::min(0.f, cam.y));
	m_world->setPosition(cam);

	bool revealed = geti("mapa") != 0;
	for (auto& n : m_nodes) {
		if (n.kind != Kind::Chest) continue;
		float dx = n.pos.x - m_pos.x, dy = n.pos.y - m_pos.y;
		n.view->setVisible(revealed || sqrtf(dx * dx + dy * dy) < 140.f);
	}
	for (auto& b : biomes()) {
		auto r = b.rect;
		if (r.containsPoint(m_pos)) { m_hudBiome->setString(b.name); break; }
	}
}

// ---------- Entrada ----------
void SupraLayer::registerWithTouchDispatcher() {
	CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

bool SupraLayer::ccTouchBegan(CCTouch* t, CCEvent*) {
	auto p = m_world->convertToNodeSpace(t->getLocation());
	m_pendingNode.clear();
	for (auto& n : m_nodes) {
		if (!n.view->isVisible()) continue;
		float dx = n.pos.x - p.x, dy = n.pos.y - p.y;
		if (sqrtf(dx * dx + dy * dy) < 45.f) {
			float px = n.pos.x - m_pos.x, py = n.pos.y - m_pos.y;
			if (sqrtf(px * px + py * py) < 90.f) { interact(n); return true; }
			m_pendingNode = n.id;
			m_target = ccp(n.pos.x, n.pos.y - 30);
			m_hasTarget = true;
			return true;
		}
	}
	m_target = p;
	m_hasTarget = true;
	return true;
}

void SupraLayer::handleKey(enumKeyCodes k, bool down) {
	int bit = 0;
	switch (k) {
		case KEY_W: case KEY_Up: bit = 1; break;
		case KEY_S: case KEY_Down: bit = 2; break;
		case KEY_A: case KEY_Left: bit = 4; break;
		case KEY_D: case KEY_Right: bit = 8; break;
		default: break;
	}
	if (bit) { if (down) m_keys |= bit; else m_keys &= ~bit; }
}
void SupraLayer::keyDown(enumKeyCodes k) { handleKey(k, true); }
void SupraLayer::keyDown(enumKeyCodes k, double) { handleKey(k, true); }
void SupraLayer::keyUp(enumKeyCodes k) { handleKey(k, false); }
void SupraLayer::keyUp(enumKeyCodes k, double) { handleKey(k, false); }

void SupraLayer::keyBackClicked() { onBack(nullptr); }
void SupraLayer::onBack(CCObject*) {
	CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, MenuLayer::scene(false)));
}

void SupraLayer::onEnter() { CCLayer::onEnter(); checkReward(); refreshHud(); }
void SupraLayer::onExit() {
	auto glm = GameLevelManager::sharedState();
	if (glm->m_levelManagerDelegate == this) glm->m_levelManagerDelegate = nullptr;
	CCLayer::onExit();
}

// ---------- Interaccion ----------
MapNode* SupraLayer::findNode(std::string const& id) {
	for (auto& n : m_nodes) if (n.id == id) return &n;
	return nullptr;
}

void SupraLayer::arrive() {
	if (m_pendingNode.empty()) return;
	if (auto n = findNode(m_pendingNode)) interact(*n);
	m_pendingNode.clear();
}

void SupraLayer::floatText(CCPoint p, std::string const& s, ccColor3B col) {
	auto l = CCLabelBMFont::create(s.c_str(), "bigFont.fnt");
	l->setScale(0.4f); l->setColor(col); l->setPosition(p);
	m_world->addChild(l, 20);
	l->runAction(CCSequence::create(CCSpawn::create(CCMoveBy::create(1.2f, ccp(0, 50)), CCFadeOut::create(1.2f), nullptr), CCRemoveSelf::create(), nullptr));
}

void SupraLayer::interact(MapNode& n) {
	switch (n.kind) {
		case Kind::Level:
		case Kind::Tower: {
			if (!n.level) {
				FLAlertLayer::create("Cargando", "Aun se estan buscando niveles de plataforma recientes. Intenta de nuevo en unos segundos.", "OK")->show();
				return;
			}
			m_pending = Pending::PlayLevel; m_pendingId = n.id;
			bool tower = n.kind == Kind::Tower;
			std::string d = fmt::format("<cy>{}</c> por {}\n{}", std::string(n.level->m_levelName), std::string(n.level->m_creatorName),
				tower ? "Torre Demoniaca: nivel de alta dificultad con jefe." : "Nivel de plataforma de la comunidad.");
			FLAlertLayer::create(this, tower ? "Torre Demoniaca" : "Nivel", d, "Cancelar", "Jugar")->show();
			break;
		}
		case Kind::Shop: promptShop(n); break;
		case Kind::Chest: openChest(n); break;
		case Kind::King: talkKing(); break;
		case Kind::Monster: poke(n); break;
	}
}

void SupraLayer::promptGate(Gate& g) {
	m_pending = Pending::UnlockGate; m_pendingId = g.id;
	std::string d = geti("llaves") > 0 ? fmt::format("Camino secreto. Usar una <cy>Llave Maestra</c> o pagar {} puntos?", g.cost)
	                                    : fmt::format("Camino secreto. Cuesta <cy>{} puntos</c> abrirlo.", g.cost);
	FLAlertLayer::create(this, "Puerta bloqueada", d, "Cancelar", "Abrir")->show();
}

void SupraLayer::promptShop(MapNode&) {
	int idx = -1;
	char const* keys[] = {"mapa", "llave_compra", "botas"};
	for (int i = 0; i < SHOP_N; i++) {
		if (i == 1 || !geti(keys[i])) { idx = i; if (i == 1 || true) break; }
	}
	if (idx < 0) { FLAlertLayer::create("Tienda", "Ya compraste todo lo que tengo.", "OK")->show(); return; }
	m_pending = Pending::BuyItem; m_pendingItem = idx;
	auto& it = SHOP[idx];
	FLAlertLayer::create(this, it.name, fmt::format("{}\nPrecio: <cy>{} baguetes</c>", it.desc, it.cost), "Cancelar", "Comprar")->show();
}

void SupraLayer::openChest(MapNode& n) {
	std::string key = "chest_" + n.id;
	if (geti(key)) { FLAlertLayer::create("Cofre", "Este cofre ya esta vacio.", "OK")->show(); return; }
	seti(key, 1);
	int r = rng() % 10;
	std::string msg;
	if (r < 5) { int v = 25 + rng() % 40; seti("puntos", geti("puntos") + v); msg = fmt::format("+{} puntos", v); }
	else if (r < 9) { int v = 3 + rng() % 6; seti("baguetes", geti("baguetes") + v); msg = fmt::format("+{} baguetes", v); }
	else { seti("llaves", geti("llaves") + 1); msg = "+1 Llave Maestra"; }
	floatText(ccp(n.pos.x, n.pos.y + 40), msg);
	n.view->setOpacity(120);
	refreshHud();
}

int SupraLayer::levelsDone() {
	int c = 0;
	for (auto& n : m_nodes) if (n.kind == Kind::Level && geti("done_" + n.id)) c++;
	return c;
}
int SupraLayer::towersDone() {
	int c = 0;
	for (auto& n : m_nodes) if (n.kind == Kind::Tower && geti("done_" + n.id)) c++;
	return c;
}

void SupraLayer::talkKing() {
	int stage = std::min(KING_N - 1, towersDone() * 2 + levelsDone() / 3);
	FLAlertLayer::create("The King", fmt::format("{}\n\n<cy>Secretos revelados: {}/{}</c>", KING_LINES[stage], stage + 1, KING_N), "OK")->show();
}

void SupraLayer::poke(MapNode& n) {
	n.view->stopAllActions();
	n.view->runAction(CCSequence::create(CCScaleTo::create(0.1f, 1.4f, 0.7f), CCEaseBounceOut::create(CCScaleTo::create(0.5f, 1.f)), nullptr));
	std::string key = "mon_" + n.id;
	if (!geti(key) && rng() % 3 == 0) {
		seti(key, 1); seti("baguetes", geti("baguetes") + 1);
		floatText(ccp(n.pos.x, n.pos.y + 30), "+1 baguete!");
		refreshHud();
	} else {
		char const* t[] = {"Grr!", "Boo!", "Blip!", "Zzz...", "Hehe"};
		floatText(ccp(n.pos.x, n.pos.y + 30), t[rng() % 5], {255, 180, 255});
	}
}

void SupraLayer::FLAlert_Clicked(FLAlertLayer*, bool btn2) {
	if (!btn2) { m_pending = Pending::None; return; }
	switch (m_pending) {
		case Pending::PlayLevel: {
			auto n = findNode(m_pendingId);
			if (n && n->level) {
				Mod::get()->setSavedValue<std::string>("pending_node", n->id);
				CCDirector::get()->pushScene(CCTransitionFade::create(0.5f, LevelInfoLayer::scene(n->level, false)));
			}
			break;
		}
		case Pending::UnlockGate: {
			for (auto& g : m_gates) {
				if (g.id != m_pendingId) continue;
				if (geti("llaves") > 0) seti("llaves", geti("llaves") - 1);
				else if (geti("puntos") >= g.cost) seti("puntos", geti("puntos") - g.cost);
				else { FLAlertLayer::create("Sin fondos", "No tienes suficientes puntos ni llaves.", "OK")->show(); break; }
				m_unlocked.insert(g.id);
				Mod::get()->setSavedValue<bool>("gate_" + g.id, true);
				g.view->setVisible(false);
				floatText(ccp(g.rect.getMidX(), g.rect.getMidY()), "Camino abierto!", {120, 255, 120});
			}
			break;
		}
		case Pending::BuyItem: {
			auto& it = SHOP[m_pendingItem];
			if (geti("baguetes") < it.cost) { FLAlertLayer::create("Sin fondos", "No tienes suficientes baguetes.", "OK")->show(); break; }
			seti("baguetes", geti("baguetes") - it.cost);
			if (m_pendingItem == 0) seti("mapa", 1);
			else if (m_pendingItem == 1) seti("llaves", geti("llaves") + 1);
			else seti("botas", 1);
			floatText(m_pos, "Comprado!", {120, 255, 120});
			break;
		}
		default: break;
	}
	m_pending = Pending::None;
	refreshHud();
}

void SupraLayer::checkReward() {
	auto id = Mod::get()->getSavedValue<std::string>("pending_node", "");
	if (id.empty()) return;
	auto n = findNode(id);
	if (!n || !n->level) return;
	if (!geti("done_" + id) && GameStatsManager::sharedState()->hasCompletedLevel(n->level)) {
		seti("done_" + id, 1);
		bool tower = n->kind == Kind::Tower;
		seti("puntos", geti("puntos") + (tower ? 100 : 20));
		seti("baguetes", geti("baguetes") + (tower ? 10 : 2));
		floatText(ccp(n->pos.x, n->pos.y + 60), tower ? "Torre superada! +100 pts" : "Nivel completado! +20 pts");
		Mod::get()->setSavedValue<std::string>("pending_node", "");
	}
}

// ---------- Niveles de la comunidad ----------
void SupraLayer::requestLevels() {
	auto glm = GameLevelManager::sharedState();
	glm->m_levelManagerDelegate = this;
	m_stage = 0;
	glm->getOnlineLevels(GJSearchObject::create(SearchType::Recent));
}

void SupraLayer::loadLevelsFinished(CCArray* levels, const char* key, int) { loadLevelsFinished(levels, key); }
void SupraLayer::loadLevelsFailed(const char* key, int) { loadLevelsFailed(key); }

void SupraLayer::loadLevelsFinished(CCArray* levels, const char*) {
	auto& dst = (m_stage == 0) ? m_recent : m_featured;
	if (levels) {
		for (auto lvl : CCArrayExt<GJGameLevel*>(levels)) {
			if (lvl && lvl->isPlatformer()) { lvl->retain(); dst.push_back(lvl); }
		}
	}
	if (m_stage == 0) {
		m_stage = 1;
		GameLevelManager::sharedState()->getOnlineLevels(GJSearchObject::create(SearchType::Featured));
	} else {
		assignLevels();
	}
}

void SupraLayer::loadLevelsFailed(const char*) {
	if (m_stage == 0) { m_stage = 1; GameLevelManager::sharedState()->getOnlineLevels(GJSearchObject::create(SearchType::Featured)); }
	else assignLevels();
}

static int scoreLevel(GJGameLevel* l, BiomeInfo const& b) {
	std::string name = std::string(l->m_levelName);
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);
	int s = 0;
	for (auto& k : b.keys) if (name.find(k) != std::string::npos) s += 2;
	return s;
}

void SupraLayer::assignLevels() {
	auto& bs = biomes();
	// Torres: primero demons destacados, luego recientes dificiles, luego cualquiera.
	std::vector<GJGameLevel*> demonPool;
	for (auto l : m_featured) if (l->m_demon.value() > 0) demonPool.push_back(l);
	for (auto l : m_recent) if (l->m_demon.value() > 0) demonPool.push_back(l);
	for (auto l : m_featured) demonPool.push_back(l);

	for (auto& n : m_nodes) {
		if (n.kind == Kind::Tower && !demonPool.empty()) {
			n.level = demonPool.front();
			demonPool.erase(demonPool.begin());
			m_recent.erase(std::remove(m_recent.begin(), m_recent.end(), n.level), m_recent.end());
		}
	}
	for (auto& n : m_nodes) {
		if (n.kind != Kind::Level || m_recent.empty()) continue;
		auto& b = bs[(int)n.biome];
		int best = -1, bi = 0;
		for (int i = 0; i < (int)m_recent.size(); i++) {
			int s = scoreLevel(m_recent[i], b);
			if (s > best) { best = s; bi = i; }
		}
		n.level = m_recent[bi];
		m_recent.erase(m_recent.begin() + bi);
	}
	for (auto& n : m_nodes) {
		if (n.level && n.text) n.text->setString(std::string(n.level->m_levelName).substr(0, 18).c_str());
	}
}
