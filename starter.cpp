#include <iostream>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace std;

int screen_x = 1200; //1136
int screen_y = 950; //896

void display_level(RenderWindow& window, char**lvl, Texture& bgTex,Sprite& bgSprite,Texture& blockTexture,Sprite& blockSprite, const int height, const int width, const int cell_size)
{
	window.draw(bgSprite);

	for (int i = 0; i < height; i += 1)
	{
		for (int j = 0; j < width; j += 1)
		{

			if (lvl[i][j] == '#')
			{
				blockSprite.setPosition(j * cell_size, i * cell_size);
				window.draw(blockSprite);
			}
		}
	}

}

void player_gravity(char** lvl, float& offset_y, float& velocityY, bool& onGround, const float& gravity, float& terminal_Velocity, float& player_x, float& player_y, const int cell_size, int& Pheight, int& Pwidth)
{
	offset_y = player_y;

	offset_y += velocityY;

	char bottom_left_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x ) / cell_size];
	char bottom_right_down = lvl[(int)(offset_y  + Pheight) / cell_size][(int)(player_x + Pwidth) / cell_size];
	char bottom_mid_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x + Pwidth / 2) / cell_size];

	if (bottom_left_down == '#' || bottom_mid_down == '#' || bottom_right_down == '#')
	{
		onGround = true;
	}
	else
	{
		player_y = offset_y;
		onGround = false;
	}

	if (!onGround)
	{
		velocityY += gravity;
		if (velocityY >= terminal_Velocity) velocityY = terminal_Velocity;
	}

	else
	{
		velocityY = 0;
	}
}


int main()
{

	RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Close | Style::Resize);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	//level specifics
	const int cell_size = 64;
	const int height = 14;
	const int width = 18;
	char** lvl;

	//level and background textures and sprites
	Texture bgTex;
	Sprite bgSprite;
	Texture blockTexture;
	Sprite blockSprite;

	bgTex.loadFromFile("Data/bg.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0,0);

	blockTexture.loadFromFile("Data/block1.png");
	blockSprite.setTexture(blockTexture);

	//Music initialisation
	Music lvlMusic;

	lvlMusic.openFromFile("Data/mus.ogg");
	lvlMusic.setVolume(20);
	lvlMusic.play();
	lvlMusic.setLoop(true);

	//player data
	float player_x = 500;
	float player_y = 150;

	float speed = 6;

	const float jumpStrength = -20; // Initial jump velocity
	const float gravity = 1;  // Gravity acceleration

	bool isJumping = false;  // Track if jumping

	bool up_collide = false;
	bool left_collide = false;
	bool right_collide = false;

	Texture PlayerTexture;
	Sprite PlayerSprite;

	bool onGround = false;

	float offset_x = 0;
	float offset_y = 0;
	float velocityY = 0;

	float terminal_Velocity = 20;

	int PlayerHeight = 102;
	int PlayerWidth = 96;

	bool up_button = false;

	char top_left = '\0';
	char top_right = '\0';
	char top_mid = '\0';

	char left_mid = '\0';
	char right_mid = '\0';

	char bottom_left = '\0';
	char bottom_right = '\0';
	char bottom_mid = '\0';

	char bottom_left_down = '\0';
	char bottom_right_down = '\0';
	char bottom_mid_down = '\0';

	char top_right_up = '\0';
	char top_mid_up = '\0';
	char top_left_up = '\0';

	PlayerTexture.loadFromFile("Data/player.png");
	PlayerSprite.setTexture(PlayerTexture);
	PlayerSprite.setScale(3,3);
	//PlayerSprite.setPosition(player_x, player_y);


	//creating level array
	lvl = new char* [height];
	for (int i = 0; i < height; i += 1)
	{
		lvl[i] = new char[width];
	}
        lvl[1][3]= '#';
        lvl[1][4]= '#';
        lvl[1][5]= '#';
        lvl[1][6]= '#';
        lvl[1][7]= '#';
        lvl[1][8]= '#';
        lvl[1][9]= '#';
        lvl[1][10]= '#';
        lvl[1][11]= '#';
        lvl[1][12]= '#';
        lvl[1][13]= '#';
        lvl[1][14]= '#';
        lvl[9][3]= '#';
        lvl[9][4]= '#';
        lvl[9][5]= '#';
        lvl[9][6]= '#';
        lvl[9][7]= '#';
        lvl[9][8]= '#';
        lvl[9][9]= '#';
        lvl[9][10]= '#';
        lvl[9][11]= '#';
        lvl[9][12]= '#';
        lvl[9][13]= '#';
        lvl[9][14]= '#';
        lvl[8][8]= '#';
        lvl[8][9]= '#';
        lvl[7][1]= '#';
        lvl[7][2]= '#';
        lvl[7][3]= '#';
        lvl[7][9]= '#';
        lvl[7][8]= '#';
        lvl[7][7]= '#';
        lvl[7][10]= '#';
        lvl[7][14]= '#';
        lvl[7][15]= '#';
        lvl[7][16]= '#';
        lvl[6][7]= '#';
        lvl[6][10]= '#';
        lvl[5][7]= '#';
        lvl[5][10]= '#';
       
        lvl[5][3]= '#';
        lvl[5][4]= '#';
        lvl[5][5]= '#';
        lvl[5][6]= '#';
        lvl[5][11]= '#';
        lvl[5][12]= '#';
        lvl[5][13]= '#';
        lvl[5][14]= '#';
        lvl[4][7]= '#';
        lvl[4][10]= '#';
        lvl[3][7]= '#';
        lvl[3][10]= '#';
        lvl[3][8]= '#';
        lvl[3][9]= '#';
        lvl[3][1]= '#';
        lvl[3][2]= '#';
        lvl[3][3]= '#';
        lvl[3][16]= '#';
        lvl[3][15]= '#';
        lvl[3][14]= '#';
        lvl[2][8]= '#';
        lvl[2][9]= '#';
	lvl[11][0] = '#';
	lvl[11][1] = '#';
	lvl[11][2] = '#';
	lvl[11][3] = '#';
	lvl[11][4] = '#';
	lvl[11][5] = '#';
	lvl[11][6] = '#';
	lvl[11][7] = '#';
	lvl[11][8] = '#';
	lvl[11][9] = '#';
	lvl[11][10] = '#';
	lvl[11][11] = '#';
	lvl[11][12] = '#';
	lvl[11][13] = '#';
	lvl[11][14] = '#';
	lvl[11][15] = '#';
	lvl[11][16] = '#';
	lvl[11][17] = '#';
	lvl[0][0] = '#';
	lvl[1][0] = '#';
	lvl[2][0] = '#';
	lvl[3][0] = '#';
	lvl[4][0] = '#';
	lvl[5][0] = '#';
	lvl[6][0] = '#';
	lvl[7][0] = '#';
	lvl[8][0] = '#';
	lvl[9][0] = '#';
	lvl[10][0] = '#';
	lvl[0][17] = '#';
	lvl[1][17] = '#';
	lvl[2][17] = '#';
	lvl[3][17] = '#';
	lvl[4][17] = '#';
	lvl[5][17] = '#';
	lvl[6][17] = '#';
	lvl[7][17] = '#';
	lvl[8][17] = '#';
	lvl[9][17] = '#';
	lvl[10][17] = '#';
	
	//lvl[10][17] = '#';
	//lvl[10][18] = '#';
	//lvl[10][19] = '#';
	//lvl[10][1] = '#';
	
	Event ev;
	//main loop
	while (window.isOpen())
	{

		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed) 
			{
				window.close();
			}

			if (ev.type == Event::KeyPressed)
			{
			}

		}
                //PlayerSprite.setPosition(player_x, player_y);
		//presing escape to close
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
                if (Keyboard::isKeyPressed(Keyboard::Left))
		{
		         
  			 player_x-=5;;
		}
		if (Keyboard::isKeyPressed(Keyboard::Right))
		{
  			player_x += 5;
		}
		if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
		{
 			velocityY = jumpStrength;
    			onGround = false;
    			isJumping = true;
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
		window.clear();

		display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);
		player_gravity(lvl,offset_y,velocityY,onGround,gravity,terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth);
		PlayerSprite.setPosition(player_x, player_y);
		window.draw(PlayerSprite);

		window.display();
	}

	//stopping music and deleting level array
	lvlMusic.stop();
	for (int i = 0; i < height; i++)
	{
		delete[] lvl[i];
	}
	delete[] lvl;

	return 0;
}

