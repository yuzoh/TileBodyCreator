#include "TiledBodyCreator.h"

USING_NS_CC;

#define PTMRATIO 64

typedef enum {
	POLYGON_FIXTURE,
	POLYLINE_FIXTURE,
    RECT_FIXTURE,
	CIRCLE_FIXTURE,
	UNKNOWN_FIXTURE
} fixtureTypes;

b2Body* TiledBodyCreator::initCollisionMap(TMXTiledMap* map, b2World* world, std::string groupname)
{
	auto collisionGroup = map->getObjectGroup(groupname);
	cocos2d::ValueVector collisionObjects = collisionGroup->getObjects();

	b2BodyDef bd;
    bd.position.Set(map->getPositionX() / 32, map->getPositionY() / 32);
	auto collisionBody = world->CreateBody(&bd);

	for(cocos2d::Value objectValue : collisionObjects)
	{
		auto fixtureShape = createFixture(objectValue.asValueMap());
		if(fixtureShape != NULL) {
			collisionBody->CreateFixture(&fixtureShape->fixture);
		}
	}
    
    return collisionBody;
}

FixtureDef* TiledBodyCreator::createFixture(cocos2d::ValueMap object)
{
	int fixtureType = RECT_FIXTURE;
	for(auto propObj : object)
	{
		if(propObj.first == "points") {
			fixtureType = POLYGON_FIXTURE;
		} else if(propObj.first == "polylinePoints") {
			fixtureType = POLYLINE_FIXTURE;
		}
	}
	if(object["type"].asString() == "Circle") {
		fixtureType = CIRCLE_FIXTURE;
	}


	if(fixtureType == POLYGON_FIXTURE) {
		return createPolygon(object);
	} else if(fixtureType == POLYLINE_FIXTURE) {
		return createPolyline(object);
	} else if(fixtureType == CIRCLE_FIXTURE) {
		return createCircle(object);
	} else if(fixtureType == RECT_FIXTURE) {
		return createRect(object);
	}
}

FixtureDef* TiledBodyCreator::createPolygon(ValueMap object)
{
	ValueVector pointsVector = object["points"].asValueVector();
	auto position = Point(object["x"].asFloat() / PTMRATIO, object["y"].asFloat() / PTMRATIO);

	b2PolygonShape *polyshape = new b2PolygonShape();
	b2Vec2 vertices[b2_maxPolygonVertices];
	int vindex = 0;

	if(pointsVector.size() > b2_maxPolygonVertices) {
		CCLOG("Skipping TMX polygon at x=%d,y=%d for exceeding %d vertices", object["x"].asInt(), object["y"].asInt(), b2_maxPolygonVertices);
		return NULL;
	}

	auto fix = new FixtureDef();

	for(Value point : pointsVector) {
		vertices[vindex].x = (point.asValueMap()["x"].asFloat() / PTMRATIO + position.x);
        vertices[vindex].y = (-point.asValueMap()["y"].asFloat() / PTMRATIO + position.y);
		vindex++;
	}

	polyshape->Set(vertices, vindex);
	fix->fixture.shape = polyshape;
    fix->setObjectName(object["name"].asString());

	return fix;
}


FixtureDef* TiledBodyCreator::createPolyline(ValueMap object)
{
	ValueVector pointsVector = object["polylinePoints"].asValueVector();
	auto position = Point(object["x"].asFloat() / PTMRATIO, object["y"].asFloat() / PTMRATIO);

	b2ChainShape *polylineshape = new b2ChainShape();
	float verticesSize = pointsVector.size()+ 1;
	b2Vec2 vertices[30];
	int vindex = 0;

	auto fix = new FixtureDef();

	for(Value point : pointsVector) {
		vertices[vindex].x = (point.asValueMap()["x"].asFloat() / PTMRATIO + position.x);
        vertices[vindex].y = (-point.asValueMap()["y"].asFloat() / PTMRATIO + position.y);
		vindex++;
	}

	polylineshape->CreateChain(vertices, vindex);
	fix->fixture.shape = polylineshape;
    fix->setObjectName(object["name"].asString());

	return fix;
}

FixtureDef* TiledBodyCreator::createCircle(ValueMap object)
{
	auto position = Point(object["x"].asFloat() / PTMRATIO, object["y"].asFloat() / PTMRATIO);
	float radius = object["width"].asFloat()/2 / PTMRATIO;

	b2CircleShape *circleshape = new b2CircleShape();
	circleshape->m_radius = radius;
	circleshape->m_p.Set(position.x + radius, position.y + radius);

	auto fix = new FixtureDef();
	fix->fixture.shape = circleshape;
    fix->setObjectName(object["name"].asString());

	return fix;
}

FixtureDef* TiledBodyCreator::createRect(ValueMap object)
{
	auto position = Point(object["x"].asFloat() / PTMRATIO, object["y"].asFloat() / PTMRATIO);
	float width = object["width"].asFloat() / PTMRATIO;
	float height = object["height"].asFloat() / PTMRATIO;

	b2PolygonShape *rectshape = new b2PolygonShape();
	b2Vec2 vertices[4];
	int vindex = 4;

	vertices[0].x = position.x + 0.0f;
	vertices[0].y = position.y + 0.0f;

	vertices[1].x = position.x + 0.0f;
	vertices[1].y = position.y + height;

	vertices[2].x = position.x + width;
	vertices[2].y = position.y + height;

	vertices[3].x = position.x + width;
	vertices[3].y = position.y + 0.0f;

	auto fix = new FixtureDef();
	rectshape->Set(vertices, vindex);
	fix->fixture.shape = rectshape;
    fix->setObjectName(object["name"].asString());

	return fix;
}