#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModulePhysics.h"

#include "p2Point.h"

#include <math.h>

ModulePhysics::ModulePhysics(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	world = NULL;
	mouse_joint = NULL;
	
}

// Destructor
ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	LOG("Creating Physics 2D environment");

	world = new b2World(b2Vec2(GRAVITY_X, -GRAVITY_Y));
	world->SetContactListener(this);

	// needed to create joints like mouse joint
	b2BodyDef bd;
	bd.position.Set(0, 0); // cualquier posición, normalmente 0,0
	ground = world->CreateBody(&bd); // cuerpo estático global

	return true;
}

update_status ModulePhysics::PreUpdate()
{

	if (mouse_joint != nullptr && (world == nullptr || mouse_joint->GetBodyB() == nullptr))
	{
		mouse_joint = nullptr;
	}
	// --- Fixed timestep setup ---
	static float accumulator = 0.0f;
	const float fixedTimeStep = 1.0f / 60.0f; // 60Hz físicas estables

	// Raylib te da el tiempo real entre frames
	float deltaTime = GetFrameTime();
	accumulator += deltaTime;

	// --- Step de física fijo ---
	while (accumulator >= fixedTimeStep)
	{
		world->Step(fixedTimeStep, 8, 3);
		accumulator -= fixedTimeStep;
	}

	// --- Colisiones tipo sensor ---
	for (b2Contact* c = world->GetContactList(); c; c = c->GetNext())
	{
		if (c->GetFixtureA()->IsSensor() && c->IsTouching())
		{
			b2BodyUserData dataA = c->GetFixtureA()->GetBody()->GetUserData();
			b2BodyUserData dataB = c->GetFixtureB()->GetBody()->GetUserData();

			PhysBody* pbA = (PhysBody*)dataA.pointer;
			PhysBody* pbB = (PhysBody*)dataB.pointer;

			if (pbA && pbB && pbA->listener)
				pbA->listener->OnCollision(pbA, pbB);
		}
	}

	return UPDATE_CONTINUE;
}

PhysBody* ModulePhysics::CreateCircle(int x, int y, int radius)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);
	b2Body* b = world->CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 20.0f;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = pbody->height = radius;

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangle(int x, int y, int width, int height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);
	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = (int)(width * 0.5f);
	pbody->height = (int)(height * 0.5f);

	return pbody;
}

PhysBody* ModulePhysics::CreateRectangleSensor(int x, int y, int width, int height)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) * 0.5f, PIXEL_TO_METERS(height) * 0.5f);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.isSensor = true;

	b->CreateFixture(&fixture);

	pbody->body = b;
	pbody->width = width;
	pbody->height = height;

	return pbody;
}

PhysBody* ModulePhysics::CreateChain(int x, int y, const int* points, int size)
{
	PhysBody* pbody = new PhysBody();

	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.allowSleep = false;
	body.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	body.userData.pointer = reinterpret_cast<uintptr_t>(pbody);

	b2Body* b = world->CreateBody(&body);

	b2ChainShape shape;
	b2Vec2* p = new b2Vec2[size / 2];

	for (int i = 0; i < size / 2; ++i)
	{
		p[i].x = PIXEL_TO_METERS(points[i * 2 + 0]);
		p[i].y = PIXEL_TO_METERS(points[i * 2 + 1]);
	}

	shape.CreateLoop(p, size / 2);

	b2FixtureDef fixture;
	fixture.shape = &shape;

	b->CreateFixture(&fixture);

	delete p;

	pbody->body = b;
	pbody->width = pbody->height = 0;

	return pbody;
}

// 
update_status ModulePhysics::PostUpdate()
{
	if (!debugEnabled && mouse_joint != nullptr)
	{
		world->DestroyJoint(mouse_joint);
		mouse_joint = nullptr;
	}

	if (debugEnabled)
	{
		b2Body* mouseSelect = nullptr;
		Vector2 mousePosition = GetMousePosition();
		b2Vec2 pMousePosition = b2Vec2(PIXEL_TO_METERS(mousePosition.x), PIXEL_TO_METERS(mousePosition.y));

		for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
		{
			b2Body* nextBody = b->GetNext();

			PhysBody* pbody = (PhysBody*)b->GetUserData().pointer;

			if (pbody != nullptr && pbody->pendingToDelete)
			{
				world->DestroyBody(b);
				delete pbody;
				b = nextBody;
				continue;
			}

			for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
			{
				if (mouse_joint == nullptr && mouseSelect == nullptr && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
				{
					if (f->TestPoint(pMousePosition))
						mouseSelect = b;
				}
			}
		}

		if (mouseSelect)
		{
			b2MouseJointDef def;
			def.bodyA = ground;
			def.bodyB = mouseSelect;
			def.target = pMousePosition;
			def.damping = 0.5f;
			def.stiffness = 20.f;
			def.maxForce = 100.f * mouseSelect->GetMass();
			mouse_joint = (b2MouseJoint*)world->CreateJoint(&def);
		}
		else if (mouse_joint && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			mouse_joint->SetTarget(pMousePosition);
		}
		else if (mouse_joint && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		{
			world->DestroyJoint(mouse_joint);
			mouse_joint = nullptr;
		}
	}

	return UPDATE_CONTINUE;
}


// Called before quitting
bool ModulePhysics::CleanUp()
{
	LOG("Destroying physics world");

	DestroyMouseJoint();
	// Delete the whole physics world!

	delete world;
	world = nullptr;

	return true;
}

void PhysBody::GetPhysicPosition(int& x, int& y) const
{
	b2Vec2 pos = body->GetPosition();
	x = METERS_TO_PIXELS(pos.x);
	y = METERS_TO_PIXELS(pos.y);
}

float PhysBody::GetRotation() const
{
	return body->GetAngle();
}

bool PhysBody::Contains(int x, int y) const
{
	b2Vec2 p(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->TestPoint(body->GetTransform(), p) == true)
			return true;
		fixture = fixture->GetNext();
	}

	return false;
}

int PhysBody::RayCast(int x1, int y1, int x2, int y2, float& normal_x, float& normal_y) const
{
	int ret = -1;

	b2RayCastInput input;
	b2RayCastOutput output;

	input.p1.Set(PIXEL_TO_METERS(x1), PIXEL_TO_METERS(y1));
	input.p2.Set(PIXEL_TO_METERS(x2), PIXEL_TO_METERS(y2));
	input.maxFraction = 1.0f;

	const b2Fixture* fixture = body->GetFixtureList();

	while (fixture != NULL)
	{
		if (fixture->GetShape()->RayCast(&output, input, body->GetTransform(), 0) == true)
		{
			// do we want the normal ?

			float fx = (float)(x2 - x1);
			float fy = (float)(y2 - y1);
			float dist = sqrtf((fx * fx) + (fy * fy));

			normal_x = output.normal.x;
			normal_y = output.normal.y;

			return (int)(output.fraction * dist);
		}
		fixture = fixture->GetNext();
	}

	return ret;
}

void ModulePhysics::BeginContact(b2Contact* contact)
{
	b2BodyUserData dataA = contact->GetFixtureA()->GetBody()->GetUserData();
	b2BodyUserData dataB = contact->GetFixtureB()->GetBody()->GetUserData();

	PhysBody* physA = (PhysBody*)dataA.pointer;
	PhysBody* physB = (PhysBody*)dataB.pointer;

	if (physA && physA->listener != NULL)
		physA->listener->OnCollision(physA, physB);

	if (physB && physB->listener != NULL)
		physB->listener->OnCollision(physB, physA);
}

PhysBody* ModulePhysics::CreateFlipper(int x, int y, int width, int height, bool left)
{
	PhysBody* flipper = new PhysBody();

	// 1. Cuerpo dinámico (la pala)
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(flipper);
	bodyDef.gravityScale = 0.0f;   // ignorar gravedad
	b2Body* body = world->CreateBody(&bodyDef);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width / 2), PIXEL_TO_METERS(height / 2));

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.friction = 0.3f;
	fixture.restitution = 0.0f;
	body->CreateFixture(&fixture);

	// 2. Crear joint motorizado con ground como bodyA
	b2RevoluteJointDef jointDef;
	jointDef.bodyA = ground;   // cuerpo estático fijo
	jointDef.bodyB = body;     // debe ser la pala dinámica
	jointDef.localAnchorA.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	float halfWidth = PIXEL_TO_METERS(width / 2);
	if (left) {
		jointDef.localAnchorB.Set(-halfWidth, 0); // extremo izquierdo
	}
	else {
		jointDef.localAnchorB.Set(halfWidth, 0);  // extremo derecho
	}
	jointDef.enableLimit = true;
	if (left) {
		jointDef.lowerAngle = -30 * DEGTORAD;
		jointDef.upperAngle = 30 * DEGTORAD;
	}
	else { // derecha
		jointDef.lowerAngle = -30 * DEGTORAD;
		jointDef.upperAngle = 30 * DEGTORAD;
	}

	jointDef.enableMotor = true;
	jointDef.maxMotorTorque = 2500.0f;

	flipper->joint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);

	// 3. Guardar el body y dimensiones
	flipper->body = body;
	flipper->width = width;
	flipper->height = height;

	return flipper;
}

void ModulePhysics::ApplyTorque(PhysBody* body, float torque)
{
	if (body && body->body)
		body->body->ApplyTorque(torque, true);
}

void ModulePhysics::DestroyBody(PhysBody* body)
{
	if (body != nullptr && body->body != nullptr && world != nullptr)
	{
		world->DestroyBody(body->body);
		body->body = nullptr;
		delete body;
	}
}

void ModulePhysics::MoveFlipper(PhysBody* flipper, float speed)
{

	if (flipper && flipper->joint)
	{
		b2RevoluteJoint* rJoint = (b2RevoluteJoint*)flipper->joint;
		rJoint->SetMotorSpeed(speed);
	}
}

PhysBody* ModulePhysics::CreateSpring(int x, int y, int width, int height)
{
	PhysBody* spring = new PhysBody();

	// Cuerpo dinámico del spring
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(spring);

	b2Body* body = world->CreateBody(&bodyDef);

	b2PolygonShape box;
	box.SetAsBox(PIXEL_TO_METERS(width) / 2, PIXEL_TO_METERS(height) / 2);

	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 1.0f;
	fixture.friction = 0.3f;
	body->CreateFixture(&fixture);

	// Prismatic joint para limitar movimiento vertical
	b2PrismaticJointDef jointDef;
	b2BodyDef staticDef;
	staticDef.type = b2_staticBody;
	staticDef.position = body->GetPosition();
	b2Body* anchor = world->CreateBody(&staticDef);

	jointDef.bodyA = anchor;
	jointDef.bodyB = body;
	jointDef.localAxisA.Set(0, 1);          // eje vertical
	jointDef.enableLimit = true;
	jointDef.lowerTranslation = 0.0f;      // posición mínima
	jointDef.upperTranslation = PIXEL_TO_METERS(100); // máximo hacia abajo
	jointDef.enableMotor = true;
	jointDef.motorSpeed = 0.0f;            // se controla en Update()
	jointDef.maxMotorForce = 2000.0f;

	spring->joint = world->CreateJoint(&jointDef);
	spring->body = body;

	return spring;
}
void ModulePhysics::CreatePolygonWall(const int* points, int size, float thickness, bool closedLoop = false)
{
	int numSegments = (size / 2) - 1;
	if (closedLoop) numSegments += 1; // añade un segmento del último al primero

	for (int i = 0; i < numSegments; ++i)
	{
		int x1 = points[(i % (size / 2)) * 2];
		int y1 = points[(i % (size / 2)) * 2 + 1];
		int x2 = points[((i + 1) % (size / 2)) * 2];
		int y2 = points[((i + 1) % (size / 2)) * 2 + 1];

		// Centro entre los dos puntos
		float cx = (x1 + x2) / 2.0f;
		float cy = (y1 + y2) / 2.0f;

		// Longitud y ángulo
		float dx = (float)(x2 - x1);
		float dy = (float)(y2 - y1);
		float length = sqrtf(dx * dx + dy * dy);
		float angle = atan2f(dy, dx);

		// Crea cuerpo rectangular estático
		PhysBody* wall = CreateRectangle(cx, cy, (int)length, (int)thickness);
		wall->body->SetType(b2_staticBody);
		wall->body->SetTransform(wall->body->GetPosition(), angle);
	}
}
void ModulePhysics::DrawDebug(ModuleRender* render)
{
	Color cNormal = { 0, 255, 0, 255 };    // Verde
	Color cSensor = { 255, 0, 0, 150 };    // Rojo semitransparente

	for (b2Body* b = world->GetBodyList(); b; b = b->GetNext())
	{
		for (b2Fixture* f = b->GetFixtureList(); f; f = f->GetNext())
		{
			Color color = f->IsSensor() ? cSensor : cNormal;

			switch (f->GetType())
			{
			case b2Shape::e_circle:
			{
				b2CircleShape* circle = (b2CircleShape*)f->GetShape();
				b2Vec2 pos = b->GetPosition();
				render->DrawCircle(
					METERS_TO_PIXELS(pos.x),
					METERS_TO_PIXELS(pos.y),
					METERS_TO_PIXELS(circle->m_radius),
					color
				);
				break;
			}

			case b2Shape::e_polygon:
			{
				b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
				int count = poly->m_count;
				b2Vec2 prev, v;
				for (int i = 0; i < count; ++i)
				{
					v = b->GetWorldPoint(poly->m_vertices[i]);
					if (i > 0)
						render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y),
							METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y),
							color);
					prev = v;
				}
				// cerrar el polígono
				v = b->GetWorldPoint(poly->m_vertices[0]);
				render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y),
					METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y),
					color);
				break;
			}

			case b2Shape::e_chain:
			{
				b2ChainShape* chain = (b2ChainShape*)f->GetShape();
				b2Vec2 prev, v;
				for (int i = 0; i < chain->m_count; ++i)
				{
					v = b->GetWorldPoint(chain->m_vertices[i]);
					if (i > 0)
						render->DrawLine(METERS_TO_PIXELS(prev.x), METERS_TO_PIXELS(prev.y),
							METERS_TO_PIXELS(v.x), METERS_TO_PIXELS(v.y),
							color);
					prev = v;
				}
				break;
			}
			case b2Shape::e_edge:
			{
				b2EdgeShape* edge = (b2EdgeShape*)f->GetShape();
				b2Vec2 v1 = b->GetWorldPoint(edge->m_vertex1);
				b2Vec2 v2 = b->GetWorldPoint(edge->m_vertex2);

				render->DrawLine(
					METERS_TO_PIXELS(v1.x),
					METERS_TO_PIXELS(v1.y),
					METERS_TO_PIXELS(v2.x),
					METERS_TO_PIXELS(v2.y),
					color
				);
				break;
			}
			}
		}
	}
	if (debugEnabled && mouse_joint != nullptr)
	{
		b2Vec2 anchor = mouse_joint->GetBodyB()->GetPosition();
		b2Vec2 target = mouse_joint->GetTarget();

		render->DrawLine(
			METERS_TO_PIXELS(anchor.x),
			METERS_TO_PIXELS(anchor.y),
			METERS_TO_PIXELS(target.x),
			METERS_TO_PIXELS(target.y),
			RED
		);
	}

}
PhysBody* ModulePhysics::CreateBumper(int x, int y, int radius, float restitution)
{
	PhysBody* bumper = new PhysBody();

	// Cuerpo estático (no se mueve, pero puede colisionar)
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(bumper);

	b2Body* body = world->CreateBody(&bodyDef);

	// Forma circular
	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);

	// Fixture con rebote alto
	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.density = 1.0f;
	fixture.restitution = restitution; // rebote
	fixture.friction = 0.0f;
	body->CreateFixture(&fixture);

	bumper->body = body;

	return bumper;
}
PhysBody* ModulePhysics::CreateTriangularBumper(int x, int y, const int* points, int count, float restitution)
{
	PhysBody* bumper = new PhysBody();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	b2Body* body = world->CreateBody(&bodyDef);

	// Convertir los puntos del array a coordenadas Box2D
	b2Vec2 verts[3];
	for (int i = 0; i < 3; ++i)
		verts[i].Set(PIXEL_TO_METERS(points[i * 2]), PIXEL_TO_METERS(points[i * 2 + 1]));

	// --- Lado activo (rebote fuerte): entre el primer y el último punto ---
	b2EdgeShape activeEdge;
	activeEdge.SetTwoSided(verts[0], verts[2]);
	b2FixtureDef fActive;
	fActive.shape = &activeEdge;
	fActive.restitution = restitution;   // rebote fuerte (ej. 1.5f)
	fActive.friction = 0.0f;
	body->CreateFixture(&fActive);

	// --- Lado pasivo 1 ---
	b2EdgeShape passiveEdge1;
	passiveEdge1.SetTwoSided(verts[0], verts[1]);
	b2FixtureDef fPassive1;
	fPassive1.shape = &passiveEdge1;
	fPassive1.restitution = 0.0f;
	fPassive1.friction = 0.3f;
	body->CreateFixture(&fPassive1);

	// --- Lado pasivo 2 ---
	b2EdgeShape passiveEdge2;
	passiveEdge2.SetTwoSided(verts[1], verts[2]);
	b2FixtureDef fPassive2;
	fPassive2.shape = &passiveEdge2;
	fPassive2.restitution = 0.0f;
	fPassive2.friction = 0.3f;
	body->CreateFixture(&fPassive2);

	bumper->body = body;
	return bumper;
}
PhysBody* ModulePhysics::CreateCollectible(int x, int y, float radius)
{
	PhysBody* collectible = new PhysBody();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;  // No se mueve
	bodyDef.position.Set(PIXEL_TO_METERS(x), PIXEL_TO_METERS(y));
	bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(collectible);
	b2Body* body = world->CreateBody(&bodyDef);

	b2CircleShape shape;
	shape.m_radius = PIXEL_TO_METERS(radius);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.isSensor = true;
	fixtureDef.restitution = 0.0f;

	body->CreateFixture(&fixtureDef);

	collectible->body = body;
	collectible->width = radius * 2;
	collectible->height = radius * 2;

	return collectible;
}
void ModulePhysics::DestroyMouseJoint()
{
	if (mouse_joint != nullptr && world != nullptr)
	{
		world->DestroyJoint(mouse_joint);
		mouse_joint = nullptr;
	}
}