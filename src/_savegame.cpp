// ================================================================
//  _savegame.cpp
// ================================================================
#include "_savegame.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>

static const int32_t MAGIC = 0x5648534D;  // 'MHSV'
static const int32_t VERSION = 1;

std::string _savegame::slotPath(int slot)
{
    char buf[32];
    sprintf(buf, "save_slot%d.dat", slot);
    return std::string(buf);
}

// ── Helpers for binary I/O ────────────────────────────────────────
static bool writeI32(FILE* f, int32_t v) { return fwrite(&v, sizeof(v), 1, f) == 1; }
static bool readI32 (FILE* f, int32_t& v){ return fread(&v, sizeof(v), 1, f) == 1; }
static bool writeF32(FILE* f, float v)   { return fwrite(&v, sizeof(v), 1, f) == 1; }
static bool readF32 (FILE* f, float& v)  { return fread(&v, sizeof(v), 1, f) == 1; }
static bool writeVec(FILE* f, const glm::vec3& v) {
    return writeF32(f, v.x) && writeF32(f, v.y) && writeF32(f, v.z);
}
static bool readVec(FILE* f, glm::vec3& v) {
    return readF32(f, v.x) && readF32(f, v.y) && readF32(f, v.z);
}

static bool writeString(FILE* f, const std::string& s)
{
    int32_t len = (int32_t)s.size();

    if (!writeI32(f, len)) return false;

    return fwrite(s.c_str(), 1, len, f) == (size_t)len;
}

static bool readString(FILE* f, std::string& s)
{
    int32_t len;

    if (!readI32(f, len)) return false;

    if (len < 0 || len > 1024) return false;

    char buffer[1025];

    if (fread(buffer, 1, len, f) != (size_t)len)
        return false;

    buffer[len] = '\0';

    s = buffer;

    return true;
}

// ================================================================
//  save
// ================================================================
bool _savegame::save(int slot, const SaveData& data)
{
    if (slot < 1 || slot > NUM_SLOTS) return false;

    FILE* f = fopen(slotPath(slot).c_str(), "wb");
    if (!f) return false;

    bool ok = true;
    ok &= writeI32(f, MAGIC);
    ok &= writeI32(f, VERSION);
    ok &= writeI32(f, data.level);
    ok &= writeI32(f, data.score);
    ok &= writeVec(f, data.playerPos);

    ok &= writeI32(f, (int32_t)data.foods.size());
    for (const auto& food : data.foods) {
        ok &= writeI32(f, food.foodType);
        ok &= writeString(f, food.texturePath);
        ok &= writeF32(f, food.displaySize);
        ok &= writeVec(f, food.pos);
        ok &= writeVec(f, food.velocity);
    }

    ok &= writeVec(f, data.cat.pos);
    ok &= writeF32(f, data.cat.facingAngle);

    ok &= writeI32(f, (int32_t)data.dogs.size());
    for (const auto& dog : data.dogs) {
        ok &= writeVec(f, dog.pos);
        ok &= writeF32(f, dog.facingAngle);
    }

    fclose(f);
    return ok;
}

// ================================================================
//  load
// ================================================================
bool _savegame::load(int slot, SaveData& out)
{
    if (slot < 1 || slot > NUM_SLOTS) return false;

    FILE* f = fopen(slotPath(slot).c_str(), "rb");
    if (!f) return false;

    int32_t magic, version;
    if (!readI32(f, magic) || magic != MAGIC ||
        !readI32(f, version) || version != VERSION) {
        fclose(f);
        return false;
    }

    int32_t levelI, scoreI;
    bool ok = true;
    ok &= readI32(f, levelI); out.level = levelI;
    ok &= readI32(f, scoreI); out.score = scoreI;
    ok &= readVec(f, out.playerPos);

    int32_t nFoods = 0;
    ok &= readI32(f, nFoods);
    out.foods.clear();
    for (int i = 0; ok && i < nFoods; i++) {
        SavedFood food;
        int32_t typeI;
        ok &= readI32(f, typeI); food.foodType = typeI;
        ok &= readString(f, food.texturePath);
        ok &= readF32(f, food.displaySize);
        ok &= readVec(f, food.pos);
        ok &= readVec(f, food.velocity);
        out.foods.push_back(food);
    }

    ok &= readVec(f, out.cat.pos);
    ok &= readF32(f, out.cat.facingAngle);

    int32_t nDogs = 0;
    ok &= readI32(f, nDogs);
    out.dogs.clear();
    for (int i = 0; ok && i < nDogs; i++) {
        SavedEnemy dog;
        ok &= readVec(f, dog.pos);
        ok &= readF32(f, dog.facingAngle);
        out.dogs.push_back(dog);
    }

    fclose(f);
    return ok;
}

// ================================================================
//  peek – cheap header read for menu display
// ================================================================
SlotInfo _savegame::peek(int slot)
{
    SlotInfo info = { false, 0, 0 };
    if (slot < 1 || slot > NUM_SLOTS) return info;

    FILE* f = fopen(slotPath(slot).c_str(), "rb");
    if (!f) return info;

    int32_t magic, version, level, score;
    if (readI32(f, magic) && magic == MAGIC &&
        readI32(f, version) && version == VERSION &&
        readI32(f, level) && readI32(f, score)) {
        info.exists = true;
        info.level = level;
        info.score = score;
    }
    fclose(f);
    return info;
}

bool _savegame::erase(int slot)
{
    if (slot < 1 || slot > NUM_SLOTS) return false;
    return remove(slotPath(slot).c_str()) == 0;
}
