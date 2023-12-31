#include <Tora/Tora.hpp>

#include <SFML/Graphics.hpp>
#include <iostream>

static constexpr tora::u64 nVertices = 4;

struct CMotion : public tora::Component {
  sf::Vector2f velocity;
};

template <tora::u64 nVerts> //
struct CMesh : public tora::Component {
  sf::Vector2f position[nVerts];
  sf::Color color[nVerts];
  sf::Vector2f texCoords[nVerts];
};

TORA_SIMPLE_ENTITY_CLASS(ERect);

struct SReappear : public tora::System {
  static void function(tora::World &world,
                       const tora::SparseVector<tora::u64> &entityIDs,
                       float screenHeight) {
    for (auto entityID : entityIDs) {
      auto &mesh = world.getComponent<CMesh<nVertices>>(entityID);

      if (mesh.position[nVertices - 1].y > screenHeight) {
        for (tora::u64 i = 0; i < nVertices; ++i) {
          mesh.position[i].y -= screenHeight;
        }
      }
    }
  }
};

struct SFallDown : public tora::System {
  static void function(tora::World &world,
                       const tora::SparseVector<tora::u64> &entityIDs,
                       float downwardAccel) {
    for (auto entityID : entityIDs) {
      auto &motion = world.getComponent<CMotion>(entityID);
      auto &mesh = world.getComponent<CMesh<nVertices>>(entityID);

      motion.velocity.y += downwardAccel;

      for (tora::u64 i = 0; i < nVertices; ++i) {
        mesh.position[i] += motion.velocity;
      }
    }
  }
};

struct SRender : public tora::System {
  static void function(tora::World &world,
                       const tora::SparseVector<tora::u64> &entityIDs,
                       sf::RenderWindow &renderer) {
    for (auto entityID : entityIDs) {
      auto &mesh = world.getComponent<CMesh<nVertices>>(entityID);
      sf::VertexArray shape(sf::TriangleStrip, nVertices);

      for (tora::u64 i = 0; i < nVertices; ++i) {
        shape[i].position = mesh.position[i];
        shape[i].color = mesh.color[i];
        shape[i].texCoords = mesh.texCoords[i];
      }

      renderer.draw(shape);
    }
  }
};

void mainLoop(tora::u64 nObjects, tora::f32 downwardAccel) {
  const float screenWidth = 1024.0f;
  const float screenHeight = 768.0f;
  const float rectSize = 10.0f;

  sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight),
                          "Falling down shapes");

  tora::World scence;

  scence.registerComponent<CMotion>();
  scence.registerComponent<CMesh<nVertices>>();

  scence.registerSystem<SFallDown, CMotion, CMesh<nVertices>>();
  scence.registerSystem<SRender, CMesh<nVertices>>();
  scence.registerSystem<SReappear, CMesh<nVertices>>();

  std::vector<tora::u64> entityIDs(nObjects);

  std::srand(static_cast<tora::u32>(std::time(nullptr)));
  for (auto &entityID : entityIDs) {
    auto entity = scence.addEntity<ERect>();
    entityID = entity.getID();

    float randX =
        10.0f + float(std::rand() / ((RAND_MAX + 1u) / (int)screenWidth - 60));

    float randY =
        10.0f + float(std::rand() / ((RAND_MAX + 1u) / (int)screenHeight - 60));

    scence.addComponents<CMotion, CMesh<nVertices>>(
        entity,
        CMotion{
            .velocity = sf::Vector2f{0.0f, 0.0f},
        },
        CMesh<nVertices>{
            .position =
                {
                    sf::Vector2f(randX + rectSize, randY + rectSize),
                    sf::Vector2f(randX, randY + rectSize),
                    sf::Vector2f(randX + rectSize, randY),
                    sf::Vector2f(randX, randY),
                },
            .color =
                {
                    sf::Color::Red,
                    sf::Color::Blue,
                    sf::Color::Green,
                    sf::Color::Yellow,
                },
            .texCoords = {},
        });
  }

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }
    window.clear();

    scence.execute<SFallDown>(downwardAccel);
    scence.execute<SRender>(window);
    scence.execute<SReappear>(screenHeight);

    window.display();
  }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
  tora::u64 nObjects = 1000;
  tora::f32 downwardAccel = 1e-5f;
  if (argc > 1) {
    nObjects = std::stoi(argv[1]);
  }
  if (argc > 2) {
    downwardAccel = std::stof(argv[2]);
  }
  mainLoop(nObjects, downwardAccel);
  return 0;
}