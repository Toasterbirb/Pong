#include <iostream>
#include "Birb2D.hpp"

enum PlayerType
{
	Player, Bot, NoOne
};

enum Side
{
	Left, Right, Top, Bottom, Up, Down, None
};

/* Variables */
static Birb::Random rand_gen;
bool GameRunning = true;
const Birb::Vector2f 	baseBallVector 	= { 6, 6 };
PlayerType 	lastCollider = PlayerType::NoOne;
Side 		lastSide = Side::None;
Birb::Font 	scoreFont;

void MirrorBallVector(Birb::Vector2f* ballVector, Side side, Side playerMovementDirection)
{
	float movementMultiplier = 1.00f;
	if (playerMovementDirection != Side::None && side == Left && side != Top && side != Bottom)
	{
		movementMultiplier = rand_gen.RandomFloat(1.10f, 1.30f);

		/* Change the ball movement direction depending on the player movement */
		ballVector->x *= -1;
		switch (playerMovementDirection)
		{
			case (Side::Up):
				if (ballVector->y > 0)
					ballVector->y *= -1 * movementMultiplier;
				break;

			case (Side::Down):
				if (ballVector->y < 0)
					ballVector->y *= -1 * movementMultiplier;
				break;

			default:
				break;
		}

		lastSide = Side::Left;
		return;

	}
	else
		movementMultiplier = 0.90f;

	/* Prevent the ball from glitching into the walls */
	if (side == lastSide)
	{
		Birb::Debug::Log("Cancelling repeating collision");
		return;
	}

	switch (side)
	{
		case (Top):
			ballVector->y *= -1 * movementMultiplier;
			break;

		case (Bottom):
			ballVector->y *= -1 * movementMultiplier;
			break;

		case (Left):
			ballVector->x *= -1;
			break;

		case (Right):
			ballVector->x *= -1;
			break;

		default:
			break;
	}

	lastSide = side;
}

Side BallScreenBoundHit(Birb::Vector2f pos, int radius, Birb::Window window)
{
	/* Top hit */
	if (pos.y - radius < 0)
		return Side::Top;

	/* Bottom hit */
	if (pos.y + radius > window.dimensions.y)
		return Side::Bottom;

	/* Left hit */
	if (pos.x - radius < 0)
		return Side::Left;

	/* Right hit */
	if (pos.x + radius > window.dimensions.x)
		return Side::Right;

	return Side::None;
}

void ResetBall(Birb::Vector2f* ballPosition, Birb::Vector2f* ballVector, Birb::Window window)
{
	ballPosition->x = window.dimensions.x / 2.00f;
	ballPosition->y = window.dimensions.y / 2.00f;
	ballVector->x = baseBallVector.x;
	ballVector->y = baseBallVector.y * rand_gen.RandomFloat(-1.5, 1.5);

	lastCollider 	= PlayerType::NoOne;
	lastSide 		= Side::None;
}

void UpdateBallCollider(Birb::Rect* collider, Birb::Vector2f ballPosition, int ballSize)
{
	collider->x = ballPosition.x - (ballSize / 2.00f);
	collider->y = ballPosition.y - (ballSize / 2.00f);
	collider->w = (float)ballSize;
	collider->h = (float)ballSize;
}

Side BallPlayerCollision(Birb::Rect playerDimensions, Birb::Rect botDimensions, Birb::Rect ballCollider)
{
	SDL_Rect player 	= playerDimensions.getSDLRect();
	SDL_Rect bot 		= botDimensions.getSDLRect();
	SDL_Rect ball 		= ballCollider.getSDLRect();

	/* Collision with either bot or player and the ball */
	if (SDL_HasIntersection(&player, &ball) && lastCollider != PlayerType::Player)
	{
		lastCollider = PlayerType::Player;
		return Side::Left;
	}
	else if (SDL_HasIntersection(&bot, &ball) && lastCollider != PlayerType::Bot)
	{
		lastCollider = PlayerType::Bot;
		return Side::Right;
	}
	else
		return Side::None;
}

int main(int argc, char **argv)
{
	Birb::Debug::Log("Starting Pong!");
	Birb::Debug::Log("Working directory: " + (std::string)argv[0]);
	std::string workdir = (std::string)argv[0];

	size_t pos = std::string::npos;

	// Search for the substring in string in a loop untill nothing is found
	while ((pos  = workdir.find("/pong") )!= std::string::npos)
	{
		// If found then erase it from string
		workdir.erase(pos, std::string("/pong").length());
		break;
	}

	Birb::Window window("Pong", Birb::Vector2int(1280, 720), 60, true);
	Birb::TimeStep timeStep;

	/* Initialize timestep */
	timeStep.Init(&window);

	/* Gameloop variables */
	SDL_Event event;
	bool holdingKey = false;
	scoreFont.LoadFont(workdir + "/res/fonts/manaspace/manaspc.ttf", 64);

	/* Ball variables */
	Birb::Vector2f 		ballVector 		= { 6, 6 };
	Birb::Vector2f 		ballPosition 	= { window.dimensions.x / 2.00f, window.dimensions.y / 2.00f };
	int 				ballSize 		= 8;
	Birb::Rect 			ballCollider;

	/* Player variables */
	int 			playerSpeed 			= 8;
	int 			playerSideOffset 		= 32;
	Birb::Rect 		playerDimensions 		= { (float)playerSideOffset, window.dimensions.y / 2.00f - 50, 10, 100 };
	Side 			playerMovementDirection = Side::None;

	int 			botMovementSpeed 	= 5;
	Birb::Rect 		botDimensions  		= playerDimensions;
	botDimensions.x = window.dimensions.x - playerDimensions.x - playerDimensions.w;

	/* Score variables */
	int playerScore = 0;
	int botScore = 0;
	Birb::Entity e_playerScore("Player score", Birb::Vector2int(window.dimensions.x / 2 - 150, 32), Birb::EntityComponent::Text("0", &scoreFont, &Birb::Colors::White));
	Birb::Entity e_botScore("Bot score", Birb::Vector2int(window.dimensions.x / 2 + 150 - 64, 32), Birb::EntityComponent::Text("0", &scoreFont, &Birb::Colors::White));

	/* Sounds */
	Birb::Audio::Init(MIX_INIT_MP3);
	Birb::Audio::SoundFile paddle_collision(workdir + "/res/sounds/paddle_collision.wav");
	Birb::Audio::SoundFile player_lose(workdir + "/res/sounds/player_lose.wav");
	Birb::Audio::SoundFile player_point(workdir + "/res/sounds/player_point.wav");

	while (GameRunning)
	{
		timeStep.Start();
		while (timeStep.Running())
		{
			bool playerMoved = false;
			while (SDL_PollEvent(&event) != 0)
			{
				window.EventTick(event, &GameRunning);

				/* Player movement */
				if (event.type == SDL_KEYDOWN)
				{
					switch (event.key.keysym.scancode)
					{
						/* Arrow keys */
						/* Up arrow */
						case (82):
							playerMovementDirection = Side::Up;
							break;

						/* Down arrow */
						case (81):
							playerMovementDirection = Side::Down;
							break;

						/* Vim keys */
						/* k */
						case (14):
							playerMovementDirection = Side::Up;
							break;

						/* j */
						case (13):
							playerMovementDirection = Side::Down;
							break;

						/* Controller */
						default:
							playerMovementDirection = Side::None;
							break;
					}
				}
				else if (event.type == SDL_KEYUP)
				{
					playerMovementDirection = Side::None;
				}
			}

			timeStep.Step();
		}

		timeStep.End();

		/* Update score position */
		e_playerScore.rect.x = window.dimensions.x / 2.00 - 150;
		e_botScore.rect.x = window.dimensions.x / 2.00 + 150 - 64;

		/* Handle player movement */
		{
			if (playerMovementDirection == Side::Up)
				playerDimensions.y -= playerSpeed;
			else if (playerMovementDirection == Side::Down)
				playerDimensions.y += playerSpeed;
		}

		/* Handle bot movement */
		{
			if (ballPosition.x > window.dimensions.x / 2.00f && lastCollider != PlayerType::Bot) // Only move the bot paddle if the ball is on its side and it hasn't hit the ball yet
			{
				if (ballPosition.y - (playerDimensions.h / 2.00f) > 0 && ballPosition.y + (playerDimensions.h / 2.00f) < window.dimensions.y)
				{
					if (ballPosition.y > botDimensions.y + (botDimensions.h / 2.00f))
						botDimensions.y += botMovementSpeed;
					else
						botDimensions.y -= botMovementSpeed;
				}
			}

			/* Update the horizontal position of the bot in case the window dimensions are changed */
			botDimensions.x = window.dimensions.x - playerDimensions.x - playerDimensions.w;
		}

		/* Ball movemement and colliders */
		{
			ballPosition.x += ballVector.x;
			ballPosition.y += ballVector.y;

			UpdateBallCollider(&ballCollider, ballPosition, ballSize);

			/* Ball hit the top or bottom. Just bounce */
			Side hitSide = BallScreenBoundHit(ballPosition, ballSize, window);
			if (hitSide == Side::Top || hitSide == Side::Bottom)
				MirrorBallVector(&ballVector, hitSide, playerMovementDirection);

			/* Ball hit the side walls. Reset the game */
			if (hitSide == Side::Left || hitSide == Side::Right)
			{
				/* Handle scoring and audio */
				if (hitSide == Side::Left)
				{
					player_lose.play();
					botScore++;
				}
				else
				{
					player_point.play();
					playerScore++;
				}

				MirrorBallVector(&ballVector, hitSide, playerMovementDirection);
				ResetBall(&ballPosition, &ballVector, window);
			}

			/* Check for paddle hits */
			hitSide = BallPlayerCollision(playerDimensions, botDimensions, ballCollider);
			if (hitSide != Side::None)
			{
				paddle_collision.play();
				MirrorBallVector(&ballVector, hitSide, playerMovementDirection);
			}
		}

		/* Render stuff */
		window.Clear();

		{
			/* Draw playfield divider */
			Birb::Render::DrawRect(Birb::Colors::White, Birb::Rect((window.dimensions.x / 2.00f) - 4, 0, 8, window.dimensions.y));

			/* Draw players */
			{
				/* Player */
				Birb::Render::DrawRect(Birb::Colors::White,
						Birb::Rect(playerDimensions.x, playerDimensions.y,
						playerDimensions.w, playerDimensions.h));

				/* Bot */

				Birb::Render::DrawRect(Birb::Colors::White,
						Birb::Rect(botDimensions.x, botDimensions.y,
						botDimensions.w, botDimensions.h));
			}

			/* Draw the ball */
			Birb::Render::DrawCircle(Birb::Colors::White,
					Birb::Vector2int(ballPosition.x, ballPosition.y),
					ballSize);

			/* Draw score */
			e_playerScore.SetText(std::to_string(playerScore));
			e_botScore.SetText(std::to_string(botScore));
			Birb::Render::DrawEntity(e_playerScore);
			Birb::Render::DrawEntity(e_botScore);
		}

		window.Display();
		/* End of rendering */
	}

	Birb::Debug::Log("Game should be closed now!");
	return 0;
}
