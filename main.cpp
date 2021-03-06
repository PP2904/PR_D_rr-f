#include <iostream>
#include <vector>
#include <numeric>
#include <fstream>
#include <chrono>
#include <random>


//Proportional Response Dynamics

// returned:
// - Equilibrium prices
// - equilibrium allocations

//Paper: Distributed Algorithms via Gradient Descent for Fisher Markets

//floor function eingefügt


// gibt es pro Gut genau eine Einheit??? -> Annahme ja! (22.10.20)

using namespace std;

class Bidder {
public:
    vector<double> valuation; //was mir ein gut wert ist
    double budget;
    vector<double> spent; //für welches gut gibt bidder was aus (summe aller elem in spent ist budget)

    friend ostream &operator<<(ostream &os, const Bidder &b);
};

ostream &operator<<(ostream &os, const Bidder &b) {
    for (int j = 0; j < b.spent.size(); ++j) {
        os << b.spent[j] << " ";
    }
    return os;
}

int random_number(int lb, int ub) {
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::mt19937 engine(seed);
    return (engine() % (ub - lb + 1)) + lb;
}

int main() {



    ofstream myfile;
    myfile.open("allocs.txt", std::ios_base::app);

    ofstream myfile2;
    myfile2.open("intgap.txt", std::ios_base::app);


    //sum_gap über alles aufsummiert
    double sum_overall = 0.0;



    //*
    // ********************************************

    //int given_iter = 5;
    int given_iter;
    cout << "Overall Repetitions: ";
    cin >> given_iter;


    //generate #goods
    //int num_goods = 4;
    int num_goods;
    int num_bidders;
    cout << "Number Goods: ";
    cin >> num_goods;

    //num_bidders
    cout << "Number Bidders: ";
    cin >> num_bidders;


    //multiplier for valuation
    double valMultiplier = 1.;

    //generate bidders with val, budget and spent_vec randomly
    //int num_bidders = 4;


    // Anzahl Iterations pro 1 Handel (1x PR_Dynamics Algorithmus ausführen)
    //int num_iterations = 10;
    int num_iterations;
    cout << "Number iterations: ";
    cin >> num_iterations;


    //********************************************

    //infos zu Parameter des aktuellen Durchlaufs
    myfile << "num_goods: " << num_goods << ", " << "num_bidder: " << num_bidders << ", " << "num_iterations: "
           << num_iterations << ", " << "repetitions: " << given_iter << ", " << "\n";

    myfile2 << "num_goods: " << num_goods << ", " << "num_bidder: " << num_bidders << ", " << "num_iterations: "
           << num_iterations << ", " << "repetitions: " << given_iter << ", " << "\n";
    myfile2 << "rounded util | frac util" << "\n";

    for (int iter = 1; iter <= given_iter; iter++) {


        double sum_gap = 0.0;

        vector<Bidder> bidders(num_bidders);
        vector<double> prices(num_goods);

        for (int k = 0; k < num_bidders; ++k) {
            bidders[k].valuation.resize(num_goods);
            //valuation pro Gut und Bidder
            for (auto &v: bidders[k].valuation) v = (random_number(1, 11) + random_number(1, 15)) * valMultiplier;
            bidders[k].budget = random_number(1, 11) + random_number(1, 31);
            bidders[k].spent.resize(num_goods, bidders[0].budget / (double) num_goods);
        }





        //wiederholung für PR_D Algorithmus
        for (int it = 0; it < num_iterations; ++it) {

            //myfile << "Iteration: " << it << "\n";


            //in jeder iteration werden die preise des guts i auf die menge der preise,
            // die jeder bidder ausgegeben hat, gesetzt
            for (int j = 0; j < num_goods; ++j) {
                prices[j] = 0;
                for (int i = 0; i < bidders.size(); ++i)
                    prices[j] += bidders[i].spent[j];

            }
            //update der valuations und spents pro bidder
            vector<vector<double>> update(bidders.size(), vector<double>(num_goods)); //
            for (int i = 0; i < bidders.size(); ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    update[i][j] = bidders[i].valuation[j] * bidders[i].spent[j] / prices[j];

                }
            }

            //new bid vector for next iteration
            for (int i = 0; i < bidders.size(); ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    bidders[i].spent[j] =
                            bidders[i].budget * update[i][j] / accumulate(update[i].begin(), update[i].end(), 0.0);

                }
            }

            //print für jeden bidder und jede iteration dessen ???
            cout << "Iteration " << it << ":\n";
            for (int i = 0; i < bidders.size(); ++i) {
                cout << "Bidder " << i << ": " << bidders[i] << endl;
            }
            cout << endl;





            //von Max utility und utility (im equilibrium sind diese gleich)

            vector<double> utility(num_bidders);
            vector<double> max_utility(num_bidders);
            for (int b = 0; b < num_bidders; ++b) {
                max_utility[b] = 0;
                for (int i = 0; i < num_goods; ++i) {
                    utility[b] +=
                            bidders[b].valuation[i] * bidders[b].spent[i] / prices[i]; //Aufpassen wenn prices[i] = 0!
                    if (max_utility[b] < bidders[b].valuation[i] / prices[i]) {
                        max_utility[b] = bidders[b].valuation[i] / prices[i];
                    }
                }

                max_utility[b] *= bidders[b].budget;
            }


            /*ofstream myfile;
            myfile.open("markets.txt", std::ios_base::app);
            */

            cout << endl;
            cout << "Fraktionales/optimales Ergebnis: ";
            //myfile2 << "Max Utility / rounded Max Utility" << "\n";
            cout << endl;
            for (int i = 0; i < num_bidders; ++i) {
                cout << "Max Utility: " << std::setprecision(4) << max_utility[i] << endl;
                //myfile2 << std::setprecision(4) << max_utility[i] << "\n";
            }



            /*** Write allocations to graph ***/
            vector<vector<double>> graph(num_bidders, vector<double>(num_goods));
            for (int i = 0; i < num_bidders; ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    graph[i][j] = bidders[i].spent[j] / prices[j];
                }
            }

            /***
             *
             * randomized rounding: wir runden alle Kantengewichte, bis auf eines, zu 0.
             * Das eine (zufällige) wird zu 1 gerundet und dann mit dem Nutzen des Guts, welches
             * über diese Kante mit dem Bidder verbunden ist, multipliziert;
             *
             *
             * ***/



            /*** wir tauschen hier bidder und goods, sodass nicht 2 Bidder das gleiche Gut zugewiesen bekommen ***/

            cout << "\n";
            cout << "Kanten ungleich 1 werden nicht gerundet: ";
            for (int j = 0; j < num_goods; ++j) {
                for (int i = 0; i < num_bidders; ++i) {
                    graph[i][j] = 0;
                }
                int num = random_number(0, (num_bidders) - 1);
                graph[num][j] = 1;
            }

            /*** print graph ***/
            for (int i = 0; i < num_bidders; ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    cout << graph[i][j];
                }
                cout << " | ";
            }
            cout << "\n";


            vector<double> rd_max_utility(num_bidders);




            /*** Gebe Werte aus. Die ungleich 0, zählen zu Max_utility_neu ***/


            //cout << "\n";
            //cout << "Filter: \n";
            for (int i = 0; i < num_bidders; ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    if (graph[i][j] == 1) {
                        //cout << "Valuation Bidder " << i << " for Good " << j << ": " << bidders[i].valuation[j]
                            // << " \n";
                        //cout << "budget bidder " << i << ": " << bidders[i].budget << "\n";
                        //cout << "price good " << j << ": " << prices[j] << "\n";
                        rd_max_utility[i] = rd_max_utility[i] + (graph[i][j] * bidders[i].valuation[j]);
                    }
                }
                if (rd_max_utility[i] <= max_utility[i]) {
                    //myfile2 << rd_max_utility[i] << " | ";
                    //myfile2 << std::setprecision(3) << max_utility[i] << "\n";
                }
            }


            cout << "\n";
            cout << "Rounded Utilities: \n";
            for (int i = 0; i < num_bidders; ++i) {
                cout << "Max Utility rounded for Bidder " << i << ": " << rd_max_utility[i] << " | ";
                //myfile2 << rd_max_utility[i] << "\n";


                //Berechnen hier die Integrality Gaps:
                if (rd_max_utility[i] <= max_utility[i]) {
                cout << "Integrality gap: " << std::setprecision(3) << rd_max_utility[i] / max_utility[i] << "\n";
                //myfile << "Integrality gap: " << std::setprecision(3) << rd_max_utility[i] / max_utility[i] << "\n";
                }

                else{
                    cout << "\n";
                }


                /* if (rd_max_utility[i] > max_utility[i]) {
                     cout << "Integrality gap: " << std::setprecision(3) << max_utility[i] / rd_max_utility[i] << "\n";
                     //myfile << "Integrality gap: " << std::setprecision(3) << max_utility[i] / rd_max_utility[i] << "\n";
                     sum_gap = sum_gap + (max_utility[i] / rd_max_utility[i]);
                 }*/

                myfile << sum_gap << ", ";

                //nach anzahl=bidder: in neue zeile schreiben
                if (i == num_bidders - 1) {
                    myfile << "\n";
                }

            }

            if(it ==(num_iterations -1)){
                for (int i = 0; i < num_bidders; ++i) {
                    if (rd_max_utility[i] <= max_utility[i]) {
                        myfile2 << rd_max_utility[i] << " | " << max_utility[i] << "\n";
                    }
                }

                myfile2 << "\n";

                for (int i = 0; i < num_bidders; ++i) {
                        myfile2 << rd_max_utility[i] << " | " << max_utility[i] << "\n";
                    }

                myfile2 << "\n";


            }



            //innerhalb iterations-schleife = 1x Experiment ausgeführt

        }


        sum_overall = sum_gap + sum_overall;

        myfile << "\n";
        myfile << sum_overall << "\n";
        myfile << "\n";



        //Innerhalb Experiment Wdh Schleife
    }


    //myfile.close();

    return 0;


}


