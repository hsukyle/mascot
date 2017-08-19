/*! \file ReachAndStay.hh
 *  Contains the ReachAndStay class. */

#ifndef REACHANDSTAY_HH_
#define REACHANDSTAY_HH_

#include "Reach.hh"
#include "Safe.hh"

using namespace helper;

namespace scots {

/*! \class ReachAndStay
 *  \brief A class (derived from base Adaptive) that does adaptive multiscale abstraction-based synthesis for a reach-and-stay specification.
 */
template<class X_type, class U_type>
class ReachAndStay: public Reach<X_type, U_type>, public Safe<X_type, U_type> {
public:
    /*! Constructor for a ReachAndStay object. */
    ReachAndStay(int dimX, double* lbX, double* ubX, double* etaX, double tau,
                 int dimU, double* lbU, double* ubU, double* etaU,
                 double* etaRatio, double tauRatio, int nint,
                 int numAbs, int readXX, int readAbs, char* logFile)
                 : Reach<X_type, U_type>(dimX, lbX, ubX, etaX, tau,
                                         dimU, lbU, ubU, etaU,
                                         etaRatio, tauRatio, nint,
                                         numAbs, readXX, readAbs, logFile),
                   Safe<X_type, U_type>(dimX, lbX, ubX, etaX, tau,
                                        dimU, lbU, ubU, etaU,
                                        etaRatio, tauRatio, nint,
                                        numAbs, readXX, readAbs, logFile),
                   Adaptive<X_type, U_type>(dimX, lbX, ubX, etaX, tau,
                                            dimU, lbU, ubU, etaU,
                                            etaRatio, tauRatio, nint,
                                            numAbs, readXX, readAbs, logFile)
    {
    }

    /*! Deconstructor for a ReachAndStay object. */
    ~ReachAndStay() {

    }

    template<class G_type, class I_type>
    void reachAndStay(G_type addG, I_type addI, int startAbs, int minToGoCoarser, int minToBeValid, int earlyBreak) {
        this->initializeSafe(addG);
        TicToc ttWhole;
        TicToc ttSafe;
        ttWhole.tic();

        // removing obstacles from transition relation
        this->removeFromTs(&(this->Os_));

        // solving safety
        ttSafe.tic();
        for (int curAbs = 0; curAbs < this->numAbs_; curAbs++) {
            this->nu(curAbs);
        }
        clog << "----------------------------------------safe: ";
        ttSafe.toc();

        checkMakeDir("Z");
        saveVec(this->Zs_, "Z/safeZ");
        checkMakeDir("C");
        saveVec(this->Cs_, "C/safeC");

        this->initializeReach(addI, addI);

        cout << "\nDomain of all safety controllers:\n";
        this->Xs_[this->numAbs_-1]->printInfo(1);

        // Safe initialized all Zs_ to BDD-1, initializeReach was BS for Gs_.
        for (int i = 0; i < this->numAbs_; i++) {
            this->Gs_[i]->symbolicSet_ = this->ddmgr_->bddZero();
            this->Zs_[i]->symbolicSet_ = this->ddmgr_->bddZero();
            this->Cs_[i]->symbolicSet_ = this->ddmgr_->bddZero();
        }
        clog << "Gs_, Zs_, Cs_ reset to BDD-0.\n";

        // Xs_[numAbs-1] contains combined domains of all safety controllers in the finest abstraction.
        this->Gs_[this->numAbs_-1]->symbolicSet_ = this->Xs_[this->numAbs_-1]->symbolicSet_;

        // Xs_ were changed during safety.
        for (int i = 0; i < this->numAbs_; i++) {
            this->Xs_[i]->addGridPoints();
        }
        clog << "Xs_ reset to full domain.\n";

        this->Gs_[this->numAbs_-1]->writeToFile("plotting/G.bdd");

        for (int i = this->numAbs_-1; i > 0; i--) {
            this->innerCoarserAligned(this->Gs_[i-1], this->Gs_[i], i-1);
            this->Gs_[i-1]->printInfo(1);
        }

        clog << "Gs_ set to projections of domain of safety controllers.\n";

        saveVec(this->Gs_, "G/G");

        // Adaptive safety changes Ts_ and TTs_. Reset.
        this->loadTs();

        this->reach(startAbs, minToGoCoarser, minToBeValid, earlyBreak);

        clog << "----------------------------------------reachAndStay: ";
        ttWhole.toc();


    }


};

}

#endif /* REACHANDSTAY_HH_ */
