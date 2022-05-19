#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <iomanip>
#include <algorithm>
const std::string blue[]{
    "lion", "dragon", "ninja", "iceman", "wolf"};
const std::string red[]{
    "iceman", "lion", "wolf", "ninja", "dragon"};
const std::string weapons[]{
    "sword", "bomb", "arrow"};
// time from start.
int hour = -1, minute = -1;
// a map from warrior to its initial life and attack_power.
std::map<std::string, std::pair<int, int>> wa_attribute;
int base_life, city_num, arrow_attack, loyalty_decrease, end_time;
class Warrior;
class Dragon;
class Ninja;
class Wolf;
class Iceman;
class Lion;
class Weapon;
class Sword;
class Arrow;
class Bomb;
class City;
class Base;
Weapon *get_weapon(int id, int force);
Warrior *get_warrior(const std::string wa_to_create, int wa_id, bool side);
void print_time();
void reset();
void run();
void lion_run();
int march_forward();
void produce_element();
void take_element();
void launch_arrow();
void use_bomb();
void fight();
void report_weapon();
std::vector<City *> cities;
Base *red_base;
Base *blue_base;

void print_time() {
    std::cout << std::setfill('0') << std::setw(3) << hour << ':';
    std::cout << std::setfill('0') << std::setw(2) << minute << ' ';
}

class City {
protected:
    int id;
    int life;
    // 1 means red, 0 means blue, 2 means none.
    int flag;
    int last_win;
    bool just_has_battle;

public:
    Warrior *wa_placed_red;
    Warrior *wa_placed_blue;
    Warrior *from_last_red;
    City(int life_, int id_, int flag_) :
        id(id_), life(life_), flag(flag_), last_win(2), just_has_battle(false),
        wa_placed_red(nullptr),
        wa_placed_blue(nullptr), from_last_red(nullptr) {
    }
    int get_element() const {
        return life;
    }
    int get_id() const {
        return id;
    }
    void gain_element(int amount) {
        life += amount;
    }
    void lose_element(int amount) {
        life -= amount;
    }
    bool have_one_red() const {
        if (wa_placed_red && !wa_placed_blue)
            return true;
        return false;
    }
    bool have_one_blue() const {
        if (wa_placed_blue && !wa_placed_red)
            return true;
        return false;
    }
    bool have_red() const {
        return wa_placed_red;
    }
    bool have_blue() const {
        return wa_placed_blue;
    }
    int get_taken() {
        int element = life;
        life = 0;
        return element;
    }
    int get_flag() const {
        return flag;
    }
    bool has_battle() const {
        return (wa_placed_blue && wa_placed_red);
    }
    void update_win(int res) {
        if (res == 2) {
            last_win = 2;
            return;
        }
        if (res == last_win && flag != res) {
            flag = res;
            if (flag == 1) {
                print_time();
                std::cout << "red flag raised in city " << id << std::endl;
            } else if (flag == 0) {
                print_time();
                std::cout << "blue flag raised in city " << id << std::endl;
            }
            return;
        }
        last_win = res;
    }
    int get_win() const {
        return last_win;
    }
    void set_battle(bool flag) {
        just_has_battle = flag;
    }
    bool just_battle() const {
        return just_has_battle;
    }
};

class Base : public City {
private:
    // 1 means red, while 0 means blue.
    bool side;
    int warrior_to_create;
    int wa_id;

public:
    Base(int life_, int id_, int flag_, bool side_) :
        City(life_, id_, flag_), side(side_), warrior_to_create(0), wa_id(1) {
    }
    bool check_life() const {
        if (side) {
            int life_consumed = wa_attribute[red[warrior_to_create]].first;
            return life >= life_consumed;
        } else {
            int life_consumed = wa_attribute[blue[warrior_to_create]].first;
            return life >= life_consumed;
        }
    }
    void make_warrior() {
        if (!check_life())
            throw(-1);
        if (side) {
            std::string wa_to_create = red[warrior_to_create];
            int life_consumed = wa_attribute[wa_to_create].first;
            life -= life_consumed;
            wa_placed_red = get_warrior(wa_to_create, wa_id, side);
            warrior_to_create = (warrior_to_create + 1) % 5;
        } else {
            std::string wa_to_create = blue[warrior_to_create];
            int life_consumed = wa_attribute[wa_to_create].first;
            life -= life_consumed;
            wa_placed_blue = get_warrior(wa_to_create, wa_id, side);
            warrior_to_create = (warrior_to_create + 1) % 5;
        }
        ++wa_id;
    }
};

class Weapon {
protected:
    std::string weapon_name;
    int attack_power;
    int usage;

public:
    Weapon(const std::string weapon_name_) :
        weapon_name(weapon_name_), attack_power(0), usage(3) {
    }
    const std::string get_name() const {
        return weapon_name;
    }
    virtual int get_attack() const {
        return attack_power;
    }
    virtual bool broken() const {
        if (weapon_name == "sword") {
            return attack_power <= 0;
        } else if (weapon_name == "arrow") {
            return usage <= 0;
        } else
            return false;
    }
    int get_usage() const {
        if (weapon_name != "arrow")
            throw(-1);
        return usage;
    }
    void use_arrow() {
        --usage;
    }
    virtual ~Weapon() = default;
    void become_blunt() {
        attack_power = attack_power * 8 / 10;
    }
};

class Sword : public Weapon {
public:
    Sword(int force) :
        Weapon("sword") {
        attack_power = force * 2 / 10;
    }
    virtual bool broken() const {
        return attack_power <= 0;
    }
};

class Arrow : public Weapon {
public:
    Arrow() :
        Weapon("arrow") {
        attack_power = arrow_attack;
    }
    virtual bool broken() const {
        return usage <= 0;
    }
};

class Bomb : public Weapon {
public:
    Bomb() :
        Weapon("bomb") {
    }
};

class Warrior {
protected:
    const std::string warrior_name;
    std::vector<Weapon> all_weapons;
    int element, id, force;
    int position;
    // 1 means red, while 0 mwans blue.
    bool side;
    bool dead_arrow;

public:
    Warrior(const std::string warrior_name_, int id_, bool side_) :
        warrior_name(warrior_name_), element(wa_attribute.at(warrior_name_).first), id(id_),
        force(wa_attribute.at(warrior_name_).second), side(side_), dead_arrow(false) {
        print_time();
        if (side) {
            position = 0;
            std::cout << "red " << warrior_name << ' ' << id << ' ' << "born" << std::endl;
        } else {
            position = city_num + 1;
            std::cout << "blue " << warrior_name << ' ' << id << ' ' << "born" << std::endl;
        }
    }
    void print_name() const {
        if (side) {
            std::cout << "red " << warrior_name << ' ' << id;
        } else {
            std::cout << "blue " << warrior_name << ' ' << id;
        }
    }
    virtual int get_force() const {
        return force;
    }
    virtual int get_element() {
        return element;
    }
    bool is_alive() const {
        return element > 0;
    }
    bool is_dead() const {
        return element <= 0;
    }
    void get_attack(int amount) {
        element -= amount;
    }
    void gain_element(int amount) {
        element += amount;
    }
    void print_initiative_attack(Warrior &other) const {
        print_time();
        print_name();
        std::cout << " attacked ";
        other.print_name();
        std::cout << " in city " << position << " with " << element << " elements and force " << force << std::endl;
    }
    void print_back_attack(Warrior &other) const {
        print_time();
        print_name();
        std::cout << " fought back against ";
        other.print_name();
        std::cout << " in city " << position << std::endl;
    }
    virtual void initiative_attack(Warrior &other) {
        int attack_power = force;
        if (!all_weapons.empty()) {
            auto wea = all_weapons.begin();
            if (wea->get_name() == "sword") {
                attack_power += wea->get_attack();
                wea->become_blunt();
                if (wea->get_attack() <= 0)
                    all_weapons.clear();
            }
        }
        other.get_attack(attack_power);
    }
    virtual void back_attack(Warrior &other) {
        print_back_attack(other);
        int attack_power = force / 2;
        if (!all_weapons.empty()) {
            auto wea = all_weapons.begin();
            if (wea->get_name() == "sword") {
                attack_power += wea->get_attack();
                wea->become_blunt();
                if (wea->get_attack() <= 0)
                    all_weapons.clear();
            }
        }
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
                gain_element(element_before);
            }
        } else {
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
            }
        }
    }
    virtual void back_attack_without_print(Warrior &other) {
        int attack_power = force / 2;
        if (!all_weapons.empty()) {
            auto wea = all_weapons.begin();
            if (wea->get_name() == "sword") {
                attack_power += wea->get_attack();
                wea->become_blunt();
                if (wea->get_attack() <= 0)
                    all_weapons.clear();
            }
        }
        other.get_attack(attack_power);
    }
    virtual void attack(Warrior &other) {
        print_initiative_attack(other);
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
            } else {
                other.print_death();
                gain_element(element_before);
            }
        } else {
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
            } else {
                other.print_death();
            }
        }
    }
    virtual void attack_without_print(Warrior &other) {
        initiative_attack(other);
        if (other.is_alive()) {
            other.back_attack_without_print(*this);
        }
    }
    bool no_weapon() const {
        return all_weapons.empty();
    }
    std::vector<Weapon> &get_all_weapons() {
        return all_weapons;
    }
    const std::string get_name() const {
        return warrior_name;
    }
    bool get_side() const {
        return side;
    }
    void check_weapons() {
        if (!all_weapons.empty()) {
            all_weapons.erase(std::remove_if(all_weapons.begin(), all_weapons.end(), [](Weapon &wea) -> bool {
                                  return wea.broken();
                              }),
                              all_weapons.end());
        }
    }
    bool has_arrow() const {
        for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
            if (wea->get_name() == "arrow")
                return true;
        }
        return false;
    }
    void use_arrow() {
        for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
            if (wea->get_name() == "arrow") {
                wea->use_arrow();
                return;
            }
        }
    }
    void die_from_arrow() {
        dead_arrow = true;
    }
    bool dead_from_arrow() const {
        return dead_arrow;
    }
    bool has_bomb() const {
        for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
            if (wea->get_name() == "bomb")
                return true;
        }
        return false;
    }
    virtual ~Warrior() = default;
    void march() {
        if (side) {
            ++position;
        } else {
            --position;
        }
    }
    virtual void win(Warrior &) {
        return;
    }
    void print_death() const {
        print_time();
        print_name();
        std::cout << " was killed in city " << position << std::endl;
    }
    virtual Warrior *copy() {
        Warrior *new_wa = new Warrior(*this);
        return new_wa;
    }
};

class Dragon : public Warrior {
private:
    double morale;

public:
    virtual Dragon *copy() {
        Dragon *new_wa = new Dragon(*this);
        return new_wa;
    }
    Dragon(int id_, bool side_) :
        Warrior("dragon", id_, side_) {
        if (side) {
            morale = static_cast<double>(red_base->get_element()) / wa_attribute.at("dragon").first;
        } else {
            morale = static_cast<double>(blue_base->get_element()) / wa_attribute.at("dragon").first;
        }
        Weapon *p = get_weapon(id % 3, force);
        all_weapons.emplace_back(*p);
        std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(2);
        std::cout << "Its morale is " << morale << std::endl;
    }
    virtual void back_attack(Warrior &other) {
        print_back_attack(other);
        int attack_power = force / 2;
        if (!all_weapons.empty()) {
            auto wea = all_weapons.begin();
            if (wea->get_name() == "sword") {
                attack_power += wea->get_attack();
                wea->become_blunt();
                if (wea->get_attack() <= 0)
                    all_weapons.clear();
            }
        }
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
                morale += 0.2;
                gain_element(element_before);
            } else {
                morale -= 0.2;
            }
        } else {
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
                morale += 0.2;
            } else {
                morale -= 0.2;
            }
        }
    }
    virtual void attack(Warrior &other) {
        print_initiative_attack(other);
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
                morale -= 0.2;
                if (is_alive() && morale > 0.8)
                    yell();
            } else {
                other.print_death();
                morale += 0.2;
                gain_element(element_before);
                if (morale > 0.8) {
                    yell();
                }
            }
        } else {
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
                morale -= 0.2;
                if (is_alive() && morale > 0.8)
                    yell();
            } else {
                other.print_death();
                morale += 0.2;
                if (morale > 0.8) {
                    yell();
                }
            }
        }
    }
    void yell() const {
        print_time();
        if (side) {
            std::cout << "red dragon " << id << " yelled in city " << position << std::endl;
        } else {
            std::cout << "blue dragon " << id << " yelled in city " << position << std::endl;
        }
    }
    virtual void win(Warrior &) {
        morale += 0.2;
    }
    bool may_yell() const {
        return morale > 0.8;
    }
};

class Ninja : public Warrior {
public:
    virtual Ninja *copy() {
        Ninja *new_wa = new Ninja(*this);
        return new_wa;
    }
    Ninja(int id_, bool side_) :
        Warrior("ninja", id_, side_) {
        Weapon *p1 = get_weapon(id % 3, force);
        Weapon *p2 = get_weapon((id + 1) % 3, force);
        all_weapons.emplace_back(*p1);
        all_weapons.emplace_back(*p2);
    }
    virtual void initiative_attack(Warrior &other) {
        int attack_power = force;
        if (!all_weapons.empty()) {
            for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
                if (wea->get_name() == "sword") {
                    attack_power += wea->get_attack();
                    wea->become_blunt();
                }
            }
        }
        other.get_attack(attack_power);
    }
    virtual void back_attack(Warrior &) {
        return;
    }
    virtual void back_attack_without_print(Warrior &) {
        return;
    }
};

class Iceman : public Warrior {
private:
    int step;

public:
    virtual Iceman *copy() {
        Iceman *new_wa = new Iceman(*this);
        return new_wa;
    }
    Iceman(int id_, bool side_) :
        Warrior("iceman", id_, side_), step(0) {
        Weapon *p = get_weapon(id % 3, force);
        all_weapons.emplace_back(*p);
    }
    virtual int get_element() {
        ++step;
        if (step % 2 == 0) {
            force += 20;
            if (element > 9) {
                element -= 9;
            } else {
                element = 1;
            }
        }
        return element;
    }
};

class Wolf : public Warrior {
private:
    void capture_weapons(Warrior &other) {
        Warrior another = other;
        if (other.no_weapon())
            return;
        if (no_weapon()) {
            all_weapons = std::move(another.get_all_weapons());
        } else {
            auto other_weapons = std::move(another.get_all_weapons());
            bool sw = false, bo = false, ar = false;
            for (auto i = all_weapons.begin(); i != all_weapons.end(); ++i) {
                if (i->get_name() == "sword") {
                    sw = true;
                } else if (i->get_name() == "bomb") {
                    bo = true;
                } else {
                    ar = true;
                }
            }
            for (auto i = other_weapons.begin(); i != other_weapons.end(); ++i) {
                std::string name = i->get_name();
                if (name == "sword" && !sw)
                    all_weapons.emplace_back(*i);
                else if (name == "arrow" && !ar)
                    all_weapons.emplace_back(*i);
                else if (name == "bomb" && !bo)
                    all_weapons.emplace_back(*i);
            }
        }
    }

public:
    Wolf(int id_, bool side_) :
        Warrior("wolf", id_, side_) {
    }
    virtual Wolf *copy() {
        Wolf *new_wa = new Wolf(*this);
        return new_wa;
    }
    virtual void initiative_attack(Warrior &other) {
        int attack_power = force;
        if (!all_weapons.empty()) {
            for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
                if (wea->get_name() == "sword") {
                    attack_power += wea->get_attack();
                    wea->become_blunt();
                }
            }
        }
        other.get_attack(attack_power);
    }
    virtual void back_attack(Warrior &other) {
        print_back_attack(other);
        int attack_power = force / 2;
        if (!all_weapons.empty()) {
            for (auto wea = all_weapons.begin(); wea != all_weapons.end(); ++wea) {
                if (wea->get_name() == "sword") {
                    attack_power += wea->get_attack();
                    wea->become_blunt();
                }
            }
        }
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
                gain_element(element_before);
                capture_weapons(other);
            }
        } else {
            other.get_attack(attack_power);
            if (other.is_dead()) {
                other.print_death();
                capture_weapons(other);
            }
        }
    }
    virtual void attack(Warrior &other) {
        print_initiative_attack(other);
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            initiative_attack(other);
            if (other.is_alive())
                other.back_attack(*this);
            else {
                other.print_death();
                gain_element(element_before);
                capture_weapons(other);
            }

        } else {
            initiative_attack(other);
            if (other.is_alive())
                other.back_attack(*this);
            else {
                capture_weapons(other);
                other.print_death();
            }
        }
    }
    virtual void win(Warrior &other) {
        capture_weapons(other);
    }
};

class Lion : public Warrior {
private:
    int loyalty;

public:
    virtual Lion *copy() {
        Lion *new_wa = new Lion(*this);
        return new_wa;
    }
    Lion(int id_, bool side_) :
        Warrior("lion", id_, side_) {
        if (side) {
            loyalty = red_base->get_element();
        } else {
            loyalty = blue_base->get_element();
        }
        std::cout << "Its loyalty is " << loyalty << std::endl;
    }
    virtual void initiative_attack(Warrior &other) {
        int attack_power = force;
        other.get_attack(attack_power);
    }
    virtual void back_attack(Warrior &other) {
        print_back_attack(other);
        int attack_power = force / 2;
        if (other.get_name() == "lion") {
            int element_before = other.get_element();
            other.get_attack(attack_power);
            if (other.is_alive()) {
                loyalty -= loyalty_decrease;
            } else {
                other.print_death();
                gain_element(element_before);
            }
        } else {
            other.get_attack(attack_power);
            if (other.is_alive()) {
                loyalty -= loyalty_decrease;
            } else {
                other.print_death();
            }
        }
    }
    virtual void attack(Warrior &other) {
        print_initiative_attack(other);
        if (other.get_name() == "lion") {
            int element_other = other.get_element();
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
                loyalty -= loyalty_decrease;
            } else {
                other.print_death();
                gain_element(element_other);
            }
        } else {
            initiative_attack(other);
            if (other.is_alive()) {
                other.back_attack(*this);
                loyalty -= loyalty_decrease;
            } else {
                other.print_death();
            }
        }
    }
    bool should_run() const {
        return loyalty <= 0;
    }
};

Weapon *get_weapon(int id, int force) {
    std::string weapon_to_get = weapons[id];
    Weapon *p;
    if (weapon_to_get == "sword") {
        p = new Sword(force);
    } else if (weapon_to_get == "arrow") {
        p = new Arrow();
    } else if (weapon_to_get == "bomb") {
        p = new Bomb();
    } else
        throw(-1);
    return p;
}

Warrior *get_warrior(const std::string wa_to_create, int wa_id, bool side) {
    Warrior *p;
    if (wa_to_create == "dragon")
        p = new Dragon(wa_id, side);
    else if (wa_to_create == "ninja")
        p = new Ninja(wa_id, side);
    else if (wa_to_create == "iceman")
        p = new Iceman(wa_id, side);
    else if (wa_to_create == "wolf")
        p = new Wolf(wa_id, side);
    else if (wa_to_create == "lion")
        p = new Lion(wa_id, side);
    return p;
}

void reset() {
    hour = -1;
    minute = 0;
    for (auto It = cities.begin(); It != cities.end(); ++It) {
        if (*It)
            delete *It;
    }
    cities.clear();
    red_base = new Base(base_life, 0, 1, true);
    blue_base = new Base(base_life, city_num + 1, 0, false);
    cities.emplace_back(red_base);
    for (int i = 1; i <= city_num; ++i) {
        cities.emplace_back(new City(0, i, 2));
    }
    cities.emplace_back(blue_base);
}

void lion_run() {
    for (auto city = cities.begin(); city != cities.end(); ++city) {
        if (city == cities.begin()) {
            Warrior *red_wa = (*city)->wa_placed_red;
            if (red_wa == nullptr)
                continue;
            if (red_wa->get_name() == "lion") {
                Lion *lion_red = dynamic_cast<Lion *>(red_wa);
                if (lion_red->should_run()) {
                    print_time();
                    red_wa->print_name();
                    std::cout << " ran away" << std::endl;
                    delete red_wa;
                    (*city)->wa_placed_red = nullptr;
                }
            }
        } else if (city == cities.end() - 1) {
            Warrior *blue_wa = (*city)->wa_placed_blue;
            if (blue_wa == nullptr)
                continue;
            if (blue_wa->get_name() == "lion") {
                Lion *lion_blue = dynamic_cast<Lion *>(blue_wa);
                if (lion_blue->should_run()) {
                    print_time();
                    lion_blue->print_name();
                    std::cout << " ran away" << std::endl;
                    delete blue_wa;
                    (*city)->wa_placed_blue = nullptr;
                }
            }
        } else {
            Warrior *red_wa = (*city)->wa_placed_red;
            if (red_wa != nullptr && red_wa->get_name() == "lion") {
                Lion *lion_red = dynamic_cast<Lion *>(red_wa);
                if (lion_red->should_run()) {
                    print_time();
                    lion_red->print_name();
                    std::cout << " ran away" << std::endl;
                    delete red_wa;
                    (*city)->wa_placed_red = nullptr;
                }
            }
            Warrior *blue_wa = (*city)->wa_placed_blue;
            if (blue_wa != nullptr && blue_wa->get_name() == "lion") {
                Lion *lion_blue = dynamic_cast<Lion *>(blue_wa);
                if (lion_blue->should_run()) {
                    print_time();
                    lion_blue->print_name();
                    std::cout << " ran away" << std::endl;
                    delete blue_wa;
                    (*city)->wa_placed_blue = nullptr;
                }
            }
        }
    }
}

// return 1 if red base is captured, 0 if blue captured, 2 if none.
int march_forward() {
    int res = 2;
    for (auto city = cities.begin(); city != cities.end(); ++city) {
        if (city == cities.begin()) {
            if (city + 1 != cities.end()) {
                Warrior *blue_wa = (*(city + 1))->wa_placed_blue;
                if (blue_wa != nullptr) {
                    int element = blue_wa->get_element();
                    int force = blue_wa->get_force();
                    print_time();
                    blue_wa->print_name();
                    std::cout << " reached red headquarter with " << element << " elements and force "
                              << force << std::endl;
                    blue_wa->march();
                    if (red_base->wa_placed_blue != nullptr) {
                        print_time();
                        std::cout << "red headquarter was taken" << std::endl;
                        res = 1;
                    } else {
                        red_base->wa_placed_blue = blue_wa;
                        (*(city + 1))->wa_placed_blue = nullptr;
                    }
                }
            }
            if (red_base->wa_placed_red != nullptr) {
                Warrior *red_wa = red_base->wa_placed_red;
                int element = red_wa->get_element();
                int force = red_wa->get_force();
                print_time();
                if (city + 2 != cities.end()) {
                    red_wa->print_name();
                    std::cout << " marched to city " << (*(city + 1))->get_id() << " with " << element << " elements and force "
                              << force << std::endl;
                } else {
                    red_wa->print_name();
                    std::cout << " reached blue headquarter with " << element << " elements and force "
                              << force << std::endl;
                    if (blue_base->wa_placed_red != nullptr) {
                        print_time();
                        std::cout << "blue headquarter was taken" << std::endl;
                        res = 0;
                    }
                }
                red_wa->march();
                (*(city + 1))->from_last_red = red_wa;
                red_base->wa_placed_red = nullptr;
            }
        } else if (city + 1 == cities.end()) {
            if (blue_base->from_last_red != nullptr) {
                blue_base->wa_placed_red = blue_base->from_last_red;
                blue_base->from_last_red = nullptr;
            }
        } else {
            if (city + 1 != cities.end()) {
                Warrior *blue_wa = (*(city + 1))->wa_placed_blue;
                if (blue_wa != nullptr) {
                    int element = blue_wa->get_element();
                    int force = blue_wa->get_force();
                    print_time();
                    blue_wa->print_name();
                    std::cout << " marched to city " << (*city)->get_id() << " with " << element
                              << " elements and force " << force << std::endl;
                    blue_wa->march();
                    (*city)->wa_placed_blue = blue_wa;
                    (*(city + 1))->wa_placed_blue = nullptr;
                }
            }
            if ((*city)->wa_placed_red != nullptr) {
                Warrior *red_wa = (*city)->wa_placed_red;
                int element = red_wa->get_element();
                int force = red_wa->get_force();
                print_time();
                if (city + 2 != cities.end()) {
                    red_wa->print_name();
                    std::cout << " marched to city " << (*(city + 1))->get_id() << " with " << element << " elements and force "
                              << force << std::endl;
                } else {
                    red_wa->print_name();
                    std::cout << " reached blue headquarter with " << element << " elements and force "
                              << force << std::endl;
                    if (blue_base->wa_placed_red != nullptr) {
                        print_time();
                        std::cout << "blue headquarter was taken" << std::endl;
                        res = 0;
                    }
                }
                red_wa->march();
                (*(city + 1))->from_last_red = red_wa;
                (*city)->wa_placed_red = nullptr;
            }
            if ((*city)->from_last_red != nullptr) {
                (*city)->wa_placed_red = (*city)->from_last_red;
                (*city)->from_last_red = nullptr;
            }
        }
    }
    return res;
}

void produce_element() {
    for (auto city = cities.begin() + 1; city < cities.end() - 1; ++city) {
        (*city)->gain_element(10);
    }
}

void take_element() {
    for (int i = 1; i < city_num + 1; ++i) {
        auto city = cities[i];
        if (city->have_one_red()) {
            Warrior *red_wa = city->wa_placed_red;
            int element = city->get_taken();
            red_base->gain_element(element);
            print_time();
            red_wa->print_name();
            std::cout << " earned " << element << " elements for his headquarter" << std::endl;
        } else if (city->have_one_blue()) {
            Warrior *blue_wa = city->wa_placed_blue;
            int element = city->get_taken();
            blue_base->gain_element(element);
            print_time();
            blue_wa->print_name();
            std::cout << " earned " << element << " elements for his headquarter" << std::endl;
        }
    }
}

void launch_arrow() {
    for (int i = 1; i < city_num + 1; ++i) {
        auto city = cities[i];
        if (city->have_red()) {
            Warrior *red_wa = city->wa_placed_red;
            red_wa->check_weapons();
            City *next_city = cities[i + 1];
            if (next_city->have_blue() && red_wa->has_arrow()) {
                Warrior *blue_wa = next_city->wa_placed_blue;
                blue_wa->get_attack(arrow_attack);
                red_wa->use_arrow();
                print_time();
                if (blue_wa->is_dead()) {
                    blue_wa->die_from_arrow();
                    red_wa->print_name();
                    std::cout << " shot and killed ";
                    blue_wa->print_name();
                    std::cout << std::endl;
                } else {
                    red_wa->print_name();
                    std::cout << " shot" << std::endl;
                }
            }
        }
        if (city->have_blue()) {
            City *next_city = cities[i - 1];
            Warrior *blue_wa = city->wa_placed_blue;
            blue_wa->check_weapons();
            if (next_city->have_red() && blue_wa->has_arrow()) {
                Warrior *red_wa = next_city->wa_placed_red;
                red_wa->get_attack(arrow_attack);
                blue_wa->use_arrow();
                print_time();
                if (red_wa->is_dead()) {
                    red_wa->die_from_arrow();
                    blue_wa->print_name();
                    std::cout << " shot and killed ";
                    red_wa->print_name();
                    std::cout << std::endl;
                } else {
                    blue_wa->print_name();
                    std::cout << " shot" << std::endl;
                }
            }
        }
    }
}

void use_bomb() {
    for (int i = 1; i < 1 + city_num; ++i) {
        City *city = cities[i];
        if (city->has_battle()) {
            Warrior *red_wa = city->wa_placed_red;
            Warrior *blue_wa = city->wa_placed_blue;
            if (red_wa->dead_from_arrow() || blue_wa->dead_from_arrow())
                continue;
            Warrior *red_ = red_wa->copy();
            Warrior *blue_ = blue_wa->copy();
            bool red_win = false;
            bool blue_win = false;
            if ((city->get_flag() == 1) || (city->get_flag() == 2 && (city->get_id() % 2))) {
                red_->attack_without_print(*blue_);
                if (red_->is_dead()) {
                    blue_win = true;
                } else if (blue_->is_dead()) {
                    red_win = true;
                }
            } else {
                blue_->attack_without_print(*red_);
                if (red_->is_dead()) {
                    blue_win = true;
                } else if (blue_->is_dead()) {
                    red_win = true;
                }
            }
            if (red_win && blue_wa->has_bomb()) {
                print_time();
                blue_wa->print_name();
                std::cout << " used a bomb and killed ";
                red_wa->print_name();
                std::cout << std::endl;
                delete city->wa_placed_red;
                city->wa_placed_red = nullptr;
                delete city->wa_placed_blue;
                city->wa_placed_blue = nullptr;
            } else if (blue_win && red_wa->has_bomb()) {
                print_time();
                red_wa->print_name();
                std::cout << " used a bomb and killed ";
                blue_wa->print_name();
                std::cout << std::endl;
                delete city->wa_placed_red;
                city->wa_placed_red = nullptr;
                delete city->wa_placed_blue;
                city->wa_placed_blue = nullptr;
            }
        }
    }
}

void fight() {
    for (int i = 1; i < city_num + 1; ++i) {
        City *city = cities[i];
        if (city->has_battle()) {
            city->set_battle(true);
            bool red_win = false;
            bool blue_win = false;
            Warrior *red_wa = city->wa_placed_red;
            Warrior *blue_wa = city->wa_placed_blue;
            if (red_wa->dead_from_arrow() && blue_wa->dead_from_arrow()) {
                delete red_wa;
                city->wa_placed_red = nullptr;
                delete blue_wa;
                city->wa_placed_blue = nullptr;
                city->set_battle(false);
                continue;
            }
            if (red_wa->dead_from_arrow()) {
                if ((city->get_flag() == 1) || (city->get_flag() == 2 && (city->get_id() % 2))) {
                    blue_wa->win(*red_wa);
                } else {
                    blue_wa->win(*red_wa);
                    if (blue_wa->get_name() == "dragon") {
                        Dragon *p = dynamic_cast<Dragon *>(blue_wa);
                        if (p->may_yell()) {
                            p->yell();
                        }
                    }
                }
                blue_win = true;
                delete red_wa;
                city->wa_placed_red = nullptr;
            } else if (blue_wa->dead_from_arrow()) {
                if ((city->get_flag() == 1) || (city->get_flag() == 2 && (city->get_id() % 2))) {
                    red_wa->win(*blue_wa);
                    if (red_wa->get_name() == "dragon") {
                        Dragon *p = dynamic_cast<Dragon *>(red_wa);
                        if (p->may_yell()) {
                            p->yell();
                        }
                    }
                } else {
                    red_wa->win(*blue_wa);
                }
                red_win = true;
                delete blue_wa;
                city->wa_placed_blue = nullptr;
            } else {
                if ((city->get_flag() == 1) || (city->get_flag() == 2 && (city->get_id() % 2))) {
                    red_wa->attack(*blue_wa);
                    if (red_wa->is_dead()) {
                        blue_win = true;
                        delete red_wa;
                        city->wa_placed_red = nullptr;
                    } else if (blue_wa->is_dead()) {
                        red_win = true;
                        delete blue_wa;
                        city->wa_placed_blue = nullptr;
                    }
                } else {
                    blue_wa->attack(*red_wa);
                    if (red_wa->is_dead()) {
                        blue_win = true;
                        delete red_wa;
                        city->wa_placed_red = nullptr;
                    } else if (blue_wa->is_dead()) {
                        red_win = true;
                        delete blue_wa;
                        city->wa_placed_blue = nullptr;
                    }
                }
            }
            if (blue_win) {
                int element = city->get_element();
                print_time();
                blue_wa->print_name();
                std::cout << " earned " << element << " elements for his headquarter" << std::endl;
                city->update_win(0);
            } else if (red_win) {
                int element = city->get_element();
                print_time();
                red_wa->print_name();
                std::cout << " earned " << element << " elements for his headquarter" << std::endl;
                city->update_win(1);
            } else {
                city->update_win(2);
            }
        } else {
            city->set_battle(false);
            if (city->have_one_blue()) {
                Warrior *blue_wa = city->wa_placed_blue;
                if (blue_wa->dead_from_arrow()) {
                    delete blue_wa;
                    city->wa_placed_blue = nullptr;
                }
            } else if (city->have_one_red()) {
                Warrior *red_wa = city->wa_placed_red;
                if (red_wa->dead_from_arrow()) {
                    delete red_wa;
                    city->wa_placed_red = nullptr;
                }
            }
        }
    }
    for (int i = 1; i < city_num + 1; ++i) {
        City *city = cities[i];
        if (city->get_win() == 0 && city->just_battle()) {
            Warrior *blue_wa = city->wa_placed_blue;
            if (blue_base->get_element() >= 8) {
                blue_wa->gain_element(8);
                blue_base->lose_element(8);
            } else {
                break;
            }
        }
    }
    for (int i = city_num + 1; i >= 0; --i) {
        City *city = cities[i];
        if (city->get_win() == 1 && city->just_battle()) {
            Warrior *red_wa = city->wa_placed_red;
            if (red_base->get_element() >= 8) {
                red_wa->gain_element(8);
                red_base->lose_element(8);
            } else {
                break;
            }
        }
    }
    for (int i = 1; i < city_num + 1; ++i) {
        City *city = cities[i];
        if (city->get_win() == 1 && city->just_battle()) {
            int element = city->get_taken();
            red_base->gain_element(element);
        } else if (city->get_win() == 0 && city->just_battle()) {
            int element = city->get_taken();
            blue_base->gain_element(element);
        }
    }
}

void report_weapon() {
    for (int i = 0; i <= city_num + 1; ++i) {
        City *city = cities[i];
        if (city->wa_placed_red != nullptr) {
            Warrior *red_wa = city->wa_placed_red;
            red_wa->check_weapons();
            auto weapons = red_wa->get_all_weapons();
            print_time();
            red_wa->print_name();
            if (weapons.empty()) {
                std::cout << " has no weapon" << std::endl;
            } else {
                bool first = true;
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "arrow") {
                        if (first) {
                            std::cout << " has arrow(" << i->get_usage() << ')';
                            first = false;
                        } else {
                            std::cout << ",arrow(" << i->get_usage() << ')';
                        }
                    }
                }
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "bomb") {
                        if (first) {
                            std::cout << " has bomb";
                            first = false;
                        } else {
                            std::cout << ",bomb";
                        }
                    }
                }
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "sword") {
                        if (first) {
                            std::cout << " has sword(" << i->get_attack() << ')';
                            first = false;
                        } else {
                            std::cout << ",sword(" << i->get_attack() << ')';
                        }
                    }
                }
                std::cout << std::endl;
            }
        }
    }
    for (int i = 0; i <= city_num + 1; ++i) {
        City *city = cities[i];
        if (city->wa_placed_blue != nullptr) {
            Warrior *blue_wa = city->wa_placed_blue;
            blue_wa->check_weapons();
            auto weapons = blue_wa->get_all_weapons();
            print_time();
            blue_wa->print_name();
            if (weapons.empty()) {
                std::cout << " has no weapon" << std::endl;
            } else {
                bool first = true;
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "arrow") {
                        if (first) {
                            std::cout << " has arrow(" << i->get_usage() << ')';
                            first = false;
                        } else {
                            std::cout << ",arrow(" << i->get_usage() << ')';
                        }
                    }
                }
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "bomb") {
                        if (first) {
                            std::cout << " has bomb";
                            first = false;
                        } else {
                            std::cout << ",bomb";
                        }
                    }
                }
                for (auto i = weapons.begin(); i != weapons.end(); ++i) {
                    if (i->get_name() == "sword") {
                        if (first) {
                            std::cout << " has sword(" << i->get_attack() << ')';
                            first = false;
                        } else {
                            std::cout << ",sword(" << i->get_attack() << ')';
                        }
                    }
                }
                std::cout << std::endl;
            }
        }
    }
}

void report_base() {
    print_time();
    std::cout << red_base->get_element() << " elements in red headquarter" << std::endl;
    print_time();
    std::cout << blue_base->get_element() << " elements in blue headquarter" << std::endl;
}

void run() {
    int case_num;
    std::cin >> case_num;
    for (int case_ = 1; case_ <= case_num; ++case_) {
        std::cin >> base_life >> city_num >> arrow_attack >> loyalty_decrease >> end_time;
        std::cin >> wa_attribute["dragon"].first >> wa_attribute["ninja"].first >> wa_attribute["iceman"].first >> wa_attribute["lion"].first >> wa_attribute["wolf"].first;
        std::cin >> wa_attribute["dragon"].second >> wa_attribute["ninja"].second >> wa_attribute["iceman"].second >> wa_attribute["lion"].second >> wa_attribute["wolf"].second;
        reset();
        std::cout << "Case " << case_ << ':' << std::endl;
        while (true) {
            ++hour;
            minute = 0;
            while (minute < 60) {
                if (minute == 0) {
                    if (red_base->check_life()) {
                        red_base->make_warrior();
                    }
                    if (blue_base->check_life()) {
                        blue_base->make_warrior();
                    }
                } else if (minute == 5) {
                    lion_run();
                } else if (minute == 10) {
                    int res = march_forward();
                    if (res == 1 || res == 0) {
                        goto End;
                    }
                } else if (minute == 20) {
                    produce_element();
                } else if (minute == 30) {
                    take_element();
                } else if (minute == 35) {
                    launch_arrow();
                } else if (minute == 38) {
                    use_bomb();
                } else if (minute == 40) {
                    fight();
                } else if (minute == 50) {
                    report_base();
                } else if (minute == 55) {
                    report_weapon();
                }
                ++minute;
                if (hour * 60 + minute > end_time)
                    goto End;
            }
        }
    End:;
    }
}

int main() {
    run();
}
