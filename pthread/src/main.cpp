#include <iostream>
#include <pthread.h>
#include <gmpxx.h>
#include <fstream>
#include <cstdlib>
#include <vector>
#include "Chrono.hpp"   

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

std::vector<std::string> primes;

void* findPrimes(void* args)
{
    std::pair<std::string, std::string>* argsPair = (std::pair<std::string, std::string>*) args;
    std::string start = (*argsPair).first;
    std::string end = (*argsPair).second;

    char primeString[150];
    mpz_t start_t;
    mpz_t end_t;
    mpz_t range;
    mpz_init(start_t);
    mpz_init(end_t);
    mpz_init(range);

    mpz_set_str (start_t, (start).c_str(), 0);
    mpz_set_str (end_t, (end).c_str(), 0);
    mpz_sub(range, end_t, start_t);

    for (unsigned int i = 0; i <= mpz_get_ui(range); i++)
        {
            if ((mpz_millerrabin(start_t, 20) != 0))
            {
                pthread_mutex_lock(&mutex);
                if (std::find(primes.begin(), primes.end(), mpz_get_str(primeString, 0, start_t)) == primes.end())
                {
                    mpz_get_str(primeString, 0, start_t);
                    primes.push_back(primeString);
                }
                pthread_mutex_unlock(&mutex);
            }
            mpz_add_ui(start_t, start_t, 1);
        }
    return NULL;
}

// foncteur utilise pour le triage du conteneur de nombre premiers trouves
// solution tiree de geeksforgeeks.com
bool comp(std::string s1, std::string s2)
{
    if (s1.size() == s2.size()) {
        return s1 < s2;
    }
    else {
        return s1.size() < s2.size();
    }
}

int main()
{
    // extraction du nom de fichier
    std::string fileName;
    std::cout << "Nom du fichier: ";
    std::cin >> fileName;
    std::cin.ignore();
    std::ifstream file;
    file.open("../src/" + fileName);
    if (!file.is_open())
    {
        std::cout << "erreur de lecture du fichier" << std::endl;
        exit(EXIT_FAILURE);
    }

    // extraction du nombre de threads
    unsigned int NB_THREADS;
    std::cout << "Nombre de threads: ";
    std::cin >> NB_THREADS;
    std::cin.ignore();

    // initialisations
    mpz_t start_t;
    mpz_t end_t;
    mpz_t range;
    mpz_t n1;
    mpz_t n2;
    mpz_init(start_t);
    mpz_init(end_t);
    mpz_init(n1);
    mpz_init(n2);
    mpz_init(range);
    char n1String [150];
    char n2String [150]; 
    unsigned int subRange;
    pthread_t threads[NB_THREADS];
    std::vector<std::pair<std::string, std::string>> threadArgs;
    Chrono chrono(false);

    // traitement ligne par ligne du fichier ouvert
    std::string line;
    while (std::getline(file, line))
    {
        auto delim = line.find(' ');

        // extraction de l'intervalle et conversion en type mpz_t
        mpz_set_str(start_t, line.substr(0, delim).c_str(), 0);
        mpz_set_str(end_t, line.substr(delim, line.size()).c_str(), 0);

        // division de l'intervalle pour fins de parallelisation
        mpz_sub(range, end_t, start_t);
        subRange = mpz_get_ui(range)/NB_THREADS;

        // calcul des valeurs limites de l'intervalle divise qui seront
        // donnees en argument aux fonctions paralleles 
        mpz_set(n1, start_t);
        mpz_set(n2, start_t);
        for (unsigned int i = 0; i < NB_THREADS; i++)
        {
            if (i == 0)
            {
                mpz_add_ui(n2, n1, subRange);
            }
            else if (i == NB_THREADS - 1)
            {
                mpz_add_ui(n1, n1, i * subRange + 1);
                mpz_set(n2, end_t);
            }
            else
            {
                mpz_add_ui(n1, start_t, i * subRange + 1);
                mpz_add_ui(n2, start_t, (i + 1) * subRange);
            }
            
            threadArgs.push_back({mpz_get_str(n1String, 0, n1), mpz_get_str(n2String, 0, n2)});
            mpz_set(n1, start_t);
            mpz_set(n2, start_t);
        }
     
        // creation des threads
        for (unsigned int i = 0; i < NB_THREADS; i++)
        {
            pthread_create(&(threads[i]), NULL, &findPrimes, &(threadArgs[i]));
        }
        chrono.resume();
        for (unsigned int i = 0; i < NB_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }
        threadArgs.clear();
        chrono.pause();
    }

    // triage et affichage des nombre premiers trouves
    std::sort(primes.begin(), primes.end(), comp);
    for (auto n : primes)
    {
        std::cout << n << std::endl;
    }
    std::cerr << "\ntemps: "<< chrono.get() << "secondes" << std::endl;
    return 0;
}