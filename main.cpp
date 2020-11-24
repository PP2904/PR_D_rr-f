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
    return (engine() % (ub-lb+1))+lb;
}

int main() {

    int given_iter = 5;

    for(int iter = 1; iter < given_iter; iter ++) {




        //generate #goods
        int num_goods = 30;


        //vector<Bidder> bidders(5);

        //multiplier for valuation
        double i = 1.;

        //generate bidders with val, budget and spent_vec randomly
        int num_bidders = 30;


        vector<Bidder> bidders(num_bidders);

        for (int k = 0; k < num_bidders; ++k) {
            bidders[k].valuation.resize(num_goods);
            //valuation pro Gut und Bidder
            for (auto &v: bidders[k].valuation) v = (random_number(0, 11) + random_number(0, 15)) * i;
            bidders[k].budget = random_number(0, 11) + random_number(0, 31);
            bidders[k].spent.resize(num_goods, bidders[0].budget / (double) num_goods);
        }
        /*//srand ( time(NULL) );
        for (int k = 0; k < num_bidders; ++k) {
            //values have no meaning, just randomizing
            double r1 = random_number(0,11);
            double r2 = random_number(0,31);
            double r3 = random_number(0,8);

            bidders[k].valuation = {(r1+r2/r3)* i, (r1+r2/r3) * i, (r1+r2/r3) * i};
            bidders[k].budget = r1+r2;
            bidders[k].spent.resize(num_goods, bidders[0].budget / (double) num_goods);

        }
    */
        int num_iterations = 20;
        vector<double> prices(num_goods);
        for (int it = 0; it < num_iterations; ++it) {

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


            ofstream myfile;
            myfile.open("markets.txt", std::ios_base::app);


            //supply umbennenen und dafür
            cout << endl;
            cout << "Fraktionales/optimales Ergebnis: ";
            cout << endl;
            for (int j = 0; j < num_goods; ++j) {
                double demand = 0;
                double supply = 0;
                for (int i = 0; i < bidders.size(); ++i) {
                    demand += bidders[i].spent[j] / prices[j];
                }
                //cout << "Demand: " << demand << endl;
                //cout << "Supply: " << prices[j] << endl;
            }

            for (int i = 0; i < num_bidders; ++i) {
                cout << "Max Utility: " << std::setprecision(4) << max_utility[i] << endl;
                myfile << "Max Utility: " << std::setprecision(4) << max_utility[i] << endl;
            }



            /*** Write allocations to graph ***/
            vector<vector<double>> graph(num_bidders, vector<double>(num_goods));
            for (int i = 0; i < num_bidders; ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    graph[i][j] = bidders[i].spent[j] / prices[j];
                }
            }

            /*** randomized rounding: wir runden alle Kantengewichte, bis auf eines, zu 0.
             * Das eine (zufällige) wird zu 1 gerundet und dann mit dem Nutzen des Guts, welches
             * über diese Kante mit dem Bidder verbunden ist, multipliziert;
             *
             *
             * ***/


            /***
             * hier können 2 bidder das gleiche Gut bekommen
                * ***/
            /*  cout << "Kanten ungleich 1 werden nicht gerundet: ";
              for(int i=0; i < num_bidders; ++i) {
                  for (int j = 0; j < num_goods; ++j) {
                      graph[i][j] = 0;
                  }
                  int num = random_number(1, (num_goods)-1);
                  graph[i][num] = 1;
              }*/

            /* cout << "Allokation/Kantengewicht: \n";
             for (int j = 0; j < num_goods; ++j) {
                 for (int i = 0; i < num_bidders; ++i) {
                     cout << graph[i][j] << " ";
                 }
                 cout << " | ";

             }*/



            /*** wir tauschen hier bidder und goods, sodass nicht 2 Bidder das gleiche Gut zugewiesen bekommen ***/

            cout << "\n";
            cout << "Kanten ungleich 1 werden nicht gerundet: ";
            for (int j = 0; j < num_goods; ++j) {
                for (int i = 0; i < num_bidders; ++i) {
                    graph[i][j] = 0;
                }
                int num = random_number(0, (num_goods) - 1);
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


            vector<double> rd_utils(num_bidders);

            double sum_gap = 0.0;


            /*** Gebe Werte aus, die ungleich 0 sind zählen zu Max_utility_neu ***/

            cout << "Filter: \n";
            for (int i = 0; i < num_bidders; ++i) {
                for (int j = 0; j < num_goods; ++j) {
                    if (graph[i][j] == 1) {
                        cout << "Valuation Bidder " << i << " for Good " << j << ": " << bidders[i].valuation[j]
                             << " \n";
                        rd_utils[i] = rd_utils[i] + (graph[i][j] * bidders[i].valuation[j]);
                    }
                }
            }

            for (int i = 0; i < num_bidders; ++i) {
                cout << "Max Utility filtered & rounden für Bidder " << i << ": " << rd_utils[i] << "\n";
                myfile << "Max Utility filtered & rounden für Bidder " << i << ": " << rd_utils[i] << "\n";

                if (rd_utils[i] <= max_utility[i]) {
                    cout << "Integrality gap: " << std::setprecision(3) << rd_utils[i] / max_utility[i] << "\n";
                    myfile << "Integrality gap: " << std::setprecision(3) << rd_utils[i] / max_utility[i] << "\n";
                    sum_gap = sum_gap + (rd_utils[i] / max_utility[i]);

                }
                if (rd_utils[i] > max_utility[i]) {
                    cout << "Integrality gap: " << std::setprecision(3) << max_utility[i] / rd_utils[i] << "\n";
                    myfile << "Integrality gap: " << std::setprecision(3) << max_utility[i] / rd_utils[i] << "\n";
                    sum_gap = sum_gap + (max_utility[i] / rd_utils[i]);
                }
            }

            myfile << endl;
            myfile << "Die dursch. Integrality Gap ist: " << (sum_gap / num_bidders) << "\n";
            myfile << "-------------------------------------------------------" << endl;
            myfile.close();


        }

    }
        return 0;


}


