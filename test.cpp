#include "pch.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

// Flower class with static and transient fields
class Flower {
public:
    static int flowerCount;
    std::string name;
    double cost;
    int freshnessLevel; // Уровень свежести
    int stemLength; // Длина стебля
    std::shared_ptr<int> transientField; // Пример поля transient

    Flower(std::string name = "", double cost = 0, int freshnessLevel = 0, int stemLength = 0)
        : name(name), cost(cost), freshnessLevel(freshnessLevel), stemLength(stemLength), transientField(std::make_shared<int>(0)) {
        ++flowerCount;
    }

    virtual void display() const {
        std::cout << "Flower: " << name << ", Cost: " << cost
            << ", Freshness: " << freshnessLevel << ", Stem Length: " << stemLength << "\n";
    }
};

int Flower::flowerCount = 0;

// Rose class
class Rose : public Flower {
public:
    Rose(double cost = 0, int freshnessLevel = 0, int stemLength = 0)
        : Flower("Rose", cost, freshnessLevel, stemLength) {}
};

// Tulip class
class Tulip : public Flower {
public:
    Tulip(double cost = 0, int freshnessLevel = 0, int stemLength = 0)
        : Flower("Tulip", cost, freshnessLevel, stemLength) {}
};

// Lily class
class Lily : public Flower {
public:
    Lily(double cost = 0, int freshnessLevel = 0, int stemLength = 0)
        : Flower("Lily", cost, freshnessLevel, stemLength) {}
};

// TextProcessor class
class TextProcessor {
private:
    std::shared_ptr<spdlog::logger> logger;

public:
    TextProcessor() {
        logger = spdlog::get("text_processor_logger");
        if (!logger) {
            logger = spdlog::stdout_color_mt("text_processor_logger");
        }
        spdlog::set_level(spdlog::level::info);
    }

    // Method to remove a substring from each line of a multiline string
    std::string removeSubstringFromLines(const std::string& text, const std::string& substring) {
        std::istringstream stream(text);
        std::ostringstream result;
        std::string line;
        bool firstLine = true;

        while (std::getline(stream, line)) {
            size_t pos;
            while ((pos = line.find(substring)) != std::string::npos) {
                line.erase(pos, substring.length());
                logger->info("Removed occurrence of substring '{}' from line.", substring);
            }
            // Trim trailing spaces
            line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());
            if (!firstLine) {
                result << "\n";
            }
            result << line;
            firstLine = false;
        }
        return result.str();
    }
};

// FlowerConnector class for serialization and deserialization
class FlowerConnector {
public:
    static void serialize(const std::vector<Flower>& flowers, const std::string& filename) {
        std::ofstream file(filename);
        for (const auto& flower : flowers) {
            file << flower.name << " " << flower.cost << " " << flower.freshnessLevel << " " << flower.stemLength << "\n";
        }
        file.close();
    }

    static std::vector<Flower> deserialize(const std::string& filename) {
        std::ifstream file(filename);
        std::vector<Flower> flowers;
        std::string name;
        double cost;
        int freshnessLevel;
        int stemLength;
        while (file >> name >> cost >> freshnessLevel >> stemLength) {
            flowers.emplace_back(name, cost, freshnessLevel, stemLength);
        }
        file.close();
        return flowers;
    }
};

// Bouquet class
class Bouquet {
public:
    std::vector<Flower> flowers;
    double totalCost;

    Bouquet() : totalCost(0) {}

    void addFlower(const Flower& flower) {
        flowers.push_back(flower);
        totalCost += flower.cost;
    }

    void sortFlowersByFreshness() {
        std::sort(flowers.begin(), flowers.end(), [](const Flower& a, const Flower& b) {
            return a.freshnessLevel > b.freshnessLevel; // Сортировка по убыванию свежести
            });
    }

    Flower findFlowerByStemLength(int minLength, int maxLength) {
        for (const Flower& flower : flowers) {
            if (flower.stemLength >= minLength && flower.stemLength <= maxLength) {
                return flower;
            }
        }
        throw std::runtime_error("No flower found in the specified stem length range.");
    }

    void displayBouquet() const {
        std::cout << "Bouquet contains:\n";
        for (const Flower& flower : flowers) {
            flower.display();
        }
        std::cout << "Total cost: " << totalCost << "\n";
    }
};

// Test class for TextProcessor
class TextProcessorTest : public ::testing::Test {
protected:
    TextProcessor* processor;

    void SetUp() override {
        processor = new TextProcessor();
    }

    void TearDown() override {
        delete processor;
        processor = nullptr;
    }
};

// Test cases for TextProcessor
TEST_F(TextProcessorTest, RemoveSubstringWithNumbersTest) {
    std::string text = "123 456 123";
    std::string substring = "123";
    std::string expected = " 456"; // Убрали пробел в конце
    ASSERT_EQ(processor->removeSubstringFromLines(text, substring), expected);
}

TEST_F(TextProcessorTest, RemoveSubstringWithPunctuationTest) {
    std::string text = "Hello, world! Hello, again!";
    std::string substring = "Hello, ";
    std::string expected = "world! again!";
    ASSERT_EQ(processor->removeSubstringFromLines(text, substring), expected);
}

TEST_F(TextProcessorTest, RemoveSubstringWithSpacesTest) {
    std::string text = "Hello   world";
    std::string substring = "   ";
    std::string expected = "Helloworld"; // Ожидаемый результат
    ASSERT_EQ(processor->removeSubstringFromLines(text, substring), expected);
}

TEST_F(TextProcessorTest, RemoveSubstringAtLineStartTest) {
    std::string text = "test This is a line.\ntest Another line.";
    std::string substring = "test ";
    std::string expected = "This is a line.\nAnother line."; // Ожидаемый результат
    ASSERT_EQ(processor->removeSubstringFromLines(text, substring), expected);
}

TEST_F(TextProcessorTest, RemoveSubstringAtLineEndTest) {
    std::string text = "This is a line test\nAnother line test";
    std::string substring = " test";
    std::string expected = "This is a line\nAnother line"; // Ожидаемый результат
    ASSERT_EQ(processor->removeSubstringFromLines(text, substring), expected);
}

// Test class for Flowers and Bouquet
class FlowerTest : public ::testing::Test {
protected:
    Flower* flower;
    Rose* rose;
    Tulip* tulip;
    Lily* lily;
    Bouquet* bouquet;

    void SetUp() override {
        Flower::flowerCount = 0; // Сбрасываем счетчик цветов перед каждым тестом

        flower = new Flower("Daisy", 2.0, 5, 25);
        rose = new Rose(5.0, 7, 40);
        tulip = new Tulip(3.0, 5, 30);
        lily = new Lily(4.0, 8, 35);
        bouquet = new Bouquet();
    }

    void TearDown() override {
        delete flower;
        delete rose;
        delete tulip;
        delete lily;
        delete bouquet;
    }
};


// Test cases for Flowers and Bouquet
TEST_F(FlowerTest, CreateFlower) {
    ASSERT_EQ(flower->name, "Daisy");
    ASSERT_EQ(flower->cost, 2.0);
    ASSERT_EQ(flower->freshnessLevel, 5);
    ASSERT_EQ(flower->stemLength, 25);
}

TEST_F(FlowerTest, AddFlowerToBouquet) {
    bouquet->addFlower(*rose);
    ASSERT_EQ(bouquet->flowers.size(), 1);
    ASSERT_EQ(bouquet->totalCost, 5.0);
}

TEST_F(FlowerTest, SortFlowersByFreshness) {
    bouquet->addFlower(*rose);
    bouquet->addFlower(*tulip);
    bouquet->addFlower(*lily);
    bouquet->sortFlowersByFreshness();
    ASSERT_EQ(bouquet->flowers.front().name, "Lily");
}

TEST_F(FlowerTest, FindFlowerByStemLength) {
    bouquet->addFlower(*rose);
    bouquet->addFlower(*tulip);
    bouquet->addFlower(*lily);
    Flower foundFlower = bouquet->findFlowerByStemLength(30, 40);
    ASSERT_EQ(foundFlower.name, "Rose");
}

TEST_F(FlowerTest, StaticFieldTest) {
    ASSERT_EQ(Flower::flowerCount, 4); // 4 цветка создано
}
TEST_F(FlowerTest, TransientFieldNotSerializedTest) {
    flower->transientField = std::make_shared<int>(123);
    FlowerConnector::serialize({ *flower }, "flower_test.txt");
    std::vector<Flower> deserializedFlowers = FlowerConnector::deserialize("flower_test.txt");
    ASSERT_EQ(deserializedFlowers.size(), 1);
    ASSERT_EQ(*(deserializedFlowers[0].transientField), 0); // Значение transient поля не должно сохраняться
}

TEST_F(FlowerTest, ModifyTransientFieldTest) {
    flower->transientField = std::make_shared<int>(123);
    ASSERT_EQ(*(flower->transientField), 123);
}

// Main function
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
