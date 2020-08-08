#include <iostream>
#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <ctime>
#include <cstdlib>


struct gameT ;
struct pointT;
/* A struct encoding a point in a two-dimensional grid */
struct pointT {
    int row, col;
};


void InitializeGame(gameT&);
void RunSimulation(gameT&);
void OpenUserFile (std::ifstream& );
void LoadWorld (gameT&, std::ifstream&);
void Pause();
pointT MakePoint(int row, int col);
void DisplayResult(gameT& game); 
void PrintWorld(gameT& game); 
void PerformAI(gameT&);
bool MoveSnake (gameT&);
bool RandomChance(double probability);
bool InWorld(pointT& pt, gameT& game);
pointT GetNextPosition(gameT& game , int dx, int dy);
void PlaceFood(gameT& game) ;
bool Crashed(pointT headPos, gameT& game);


template <typename T> void print_vector(const std::vector<T>& vec);
std::string GetLine ();

/* Number of food pellet that mus be eaten to win. */
const int kMaxFood = 20;

/* Constants for the different tile types */
const char kEmptyTile = ' ';
const char kWallTile  = '#';
const char kFoodTile  = '$';
const char kSnakeTile  = '*';
const std::string kClearCommand = "clear";


/* A struct containing relevant game information */
struct gameT {
    std::vector <std::string> world;   // The playing field
    int numRows,  numCols;             // Size of the playing field
 
    std::deque <pointT> snake;         // The snake body
    int dx, dy;                        // The snake direction

    int numEaten;                      // How much food we've eaten.
};

/* The main program.  Initializes the world,  the runs the simulation */

int main () {
    
    // CREAMOS LA ESTRUCTURA QUE VA A CONTENER EL JUEGO
    gameT game;
    
    // CARGAMOS Y INICIALIZAMOS EL JUEGO
    InitializeGame (game);
    
    // EJECUTAMOS TODO EL JUEGO USANDO UNA INTELIGENCIA ARTIFICIAL AI
    RunSimulation (game);

    return 0;

}




void InitializeGame(gameT& game) {
    /* 
    *  Seed the randomizer. 
    *  The static_cast converts the result of time(NULL)
    *  from time_t to the unsigned int required by srand. 
    *  This line is idiomatic C++.
    * 
    */
   
    // INICIALIZAMOS LA SEMILLA DE LOS NUMERO ALEATORIOS
    srand( static_cast<unsigned int>(time(NULL)) );
    // CREAMOS UN OBJETO PARA CARGAR EL ARCHIVO
    std::ifstream input;
    // ABRIMOS EL ARCHIVO DEL USUARIO
    OpenUserFile(input);
    // CARGAMOS EL MUNDO EN LA MEMORIA
    LoadWorld (game, input);

}






void OpenUserFile (std::ifstream& input) {

    // ITERA HASTA QUE EL USUARIO INGRESA UN NOMBRE VALIDO DE ARCHIVO
    while (true) {
        std::cout << "Enter filename";
        std::string filename = GetLine();

        input.open (filename.c_str());  
        if(input.is_open()) break;

        std::cout << "Sorry, i cant find the file " <<filename <<std::endl;
        input.clear();    // Limpia las banderas de l stream
    }

    
}







void LoadWorld(gameT& game,  std::ifstream& input) {

    // RECUPERAMOS EL VALOR DEL NUMERO DE FILAS Y COLUMNAS
    input >> game.numRows >> game.numCols;
    
    // REDIMENCIONAMOS EL VECTOR WORLD PARA QUE ALOJE TODOS LOS STRINGS NECESARIOS
    game.world.resize(game.numRows);

    // RECUPERAMOS LA DIRECCION INICIAL DE LA CULEBRA
    input >>game.dx >> game.dy;

    // ELIMINAMOS UN FINAL DE LINEA REMANENTE
    std::string dummy;
    getline(input, dummy);

    // CARGAMOS EL MUNDO DESDE EL ARCHIVO,  HASTA LA ESTRUCTURA game.world
    for (int row = 0; row < game.numRows; ++row) {

       std::getline(input,   game.world[row]);

       // BUSCAMOS LA POSICION INICIAL DEL ASTERISCO PARA ASIGNARLA A LA SNAKE
       int col = game.world[row].find(kSnakeTile); 

       if(col != std::string::npos) {
       
            game.snake.push_back(MakePoint(row, col));

        }

        game.numEaten = 0;
    }


    //print_vector(game.world);
    //   std::cout <<game.world[0]<<std::endl;
    // std::cout <<game.world[1]<<std::endl;
    //  std::cout <<game.world[2]<<std::endl;

}// End LoadWorld








void RunSimulation(gameT& game) {
    /* Keep looping while we haven't eaten too much*/

    while (game.numEaten < kMaxFood) {
        // MUESTRA EL MUNDO 
        PrintWorld(game);          // Display the board
        
        // 
        PerformAI(game);          // Have the AI coose an action

        if (!MoveSnake (game)) {   // Move the snake and stop if we crashed
            break;
        }
        
        Pause();                   // Pause so we can see what's going on

    }

    DisplayResult(game);            // Tell the user what happend

}






bool MoveSnake(gameT& game) {
    pointT nextHead = GetNextPosition(game, game.dx, game.dy);
    
    if(Crashed(nextHead, game))
        return false;
        
    bool isFood = (game.world[nextHead.row][nextHead.col] == kFoodTile);

    game.world[nextHead.row][nextHead.col] = kSnakeTile; 
    game.snake.push_front(nextHead);

    if(!isFood) {
        game.world[game.snake.back().row][game.snake.back().col] = kEmptyTile; 
        game.snake.pop_back();
    } else { ++game.numEaten;
        PlaceFood(game);
    }

    return true;

} // End MoveSnake()





void PlaceFood(gameT& game) {

        while(true) {

            int row = rand() % game.numRows; 
            int col = rand() % game.numCols;

            /* If the specified position is empty, place the food there. */
            if(game.world[row][col] == kEmptyTile) { 
                game.world[row][col] = kFoodTile; 
                return;
            }

        }

} // End PlaceFood()


const double kWaitTime = 0.1; // Pause 0.1 seconds between frames 

void Pause() {
    clock_t startTime = clock(); // clock_t is a type which holds clock ticks.
    
    /* This loop does nothing except loop and check how much time is left.
     * Note that we have to typecast startTime from clock_t to double so 
     * that the division is correct. The static_cast<double>(...) syntax 
     * is the preferred C++ way of performing a typecast of this sort;
     * see the chapter on
     * inheritance for more information. */
    while(static_cast<double>  (clock() - startTime) / CLOCKS_PER_SEC < kWaitTime);
}







void PrintWorld(gameT& game) {
    
    system(kClearCommand.c_str());

    for(int row = 0; row < game.numRows; ++row) {
        std::cout << game.world[row] << std::endl;
    }
        
    std::cout << "Food eaten: " << game.numEaten << std::endl;
}





void DisplayResult(gameT& game) {
    PrintWorld(game); 
    if(game.numEaten == kMaxFood)
        std::cout << "The snake ate enough food and wins!" << std::endl; 
    else
        std::cout << "Oh no! The snake crashed!" << std::endl;
}





const double kTurnRate = 0.2; // 20% chance to turn each step. 
void PerformAI(gameT& game) {

    /* Figure out where we will be after we move this turn. */
    pointT nextHead = GetNextPosition(game , game.dx, game.dy);

    /* If that hits a wall or we randomly decide to, turn the snake. */
    
    if(Crashed(nextHead, game)  ||  RandomChance(kTurnRate)) {
        
        int leftDx = -game.dy; 
        int leftDy = game.dx;


        int rightDx = game.dy; 
        int rightDy = -game.dx;


        /* Check if turning left or right will cause us to crash. */
        bool canLeft  = !Crashed( GetNextPosition(game, leftDx, leftDy),  game  );
        bool canRight = !Crashed( GetNextPosition(game, rightDx, rightDy), game );

        bool willTurnLeft = false; 
        if(!canLeft && !canRight)
            return; // If we can't turn, don't turn. 
        else if(canLeft && !canRight)
            willTurnLeft = true; // If we must turn left, do so. 
        else if(!canLeft && canRight)
            willTurnLeft = false; // If we must turn right, do so. 
        else
            willTurnLeft = RandomChance(0.5); // Else pick randomly
        /* ... */

        game.dx = willTurnLeft? leftDx : rightDx;
        game.dy = willTurnLeft? leftDy : rightDy;


    } 
}





pointT GetNextPosition(gameT& game , int dx, int dy) { 
    /* Get the head position. */ 
    pointT result = game.snake.front();

    /* Increment the head position by the current direction. */
    result.row += dy; 
    result.col += dx; 

    return result;
}







bool Crashed(pointT headPos, gameT& game) {
    return !InWorld(headPos, game) || 
            game.world[headPos.row][headPos.col] == kSnakeTile || 
            game.world[headPos.row][headPos.col] == kWallTile;
}



bool InWorld(pointT& pt, gameT& game) {
    return  pt.col >= 0 && 
            pt.row >= 0 && 
            pt.col < game.numCols && 
            pt.row < game.numRows;
}



bool RandomChance(double probability) {
    return (rand() / (RAND_MAX + 1.0)) < probability;
}


// generic function to print any kind of vector
template <typename T>
void print_vector(const std::vector<T>& vec) {

   // std::cout << "{";
    for (auto iter = vec.begin();   iter != vec.end();    ++iter) {
        std::cout << *iter;
       // if (iter + 1 != vec.end()) std::cout << ", ";
    }
   // std::cout << "}\n";

}


std::string GetLine () {
    std::string result;
    std::getline (std::cin , result );
    return result;
}



pointT MakePoint(int row, int col) { 
    pointT result;
    result.row = row;
    result.col = col;
    return result; 
}