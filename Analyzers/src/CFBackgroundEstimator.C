#include "CFBackgroundEstimator.h"

CFBackgroundEstimator::CFBackgroundEstimator()
{

}

void CFBackgroundEstimator::ReadHistograms(){

  TString datapath = getenv("DATA_DIR");
  datapath = datapath+"/"+TString::Itoa(DataYear,10)+"/CFRate/";

  string elline;
  ifstream in(datapath+"/histmap_Electron.txt");
  while(getline(in,elline)){
    std::istringstream is( elline );
    TString a,b,c,d,e;
    is >> a; // <ID>
    is >> b; // <rootfilename>
    TString tfileName = datapath+"/"+b;
    TFile *file = new TFile(tfileName);
    if( file->IsZombie()){
      cout<<"There in no file: "<<tfileName<<", check histmap_Electron.txt, exiting.........."<<endl;
      exit(-1);
    }
    TList *histlist = file->GetListOfKeys();
    for(int i=0;i<histlist->Capacity();i++){
      TString this_cfname = histlist->At(i)->GetName();
      map_hist_Electron[a+"_"+this_cfname] = (TH1D *)file->Get(this_cfname);
      //cout << "[CFBackgroundEstimator::CFBackgroundEstimator] map_hist_Electron : " << a+"_"+this_cfname << endl;
    }
  }

  string elline2;
  ifstream in2(datapath+"/histmap_Muon.txt");
  while(getline(in2,elline2)){
    std::istringstream is( elline2 );
    TString a,b,c,d,e;
    is >> a; // <ID>
    is >> b; // <rootfilename>
    TString tfileName = datapath+"/"+b;
    TFile *file = new TFile(tfileName);
    if( file->IsZombie()){
      cout<<"There in no file: "<<tfileName<<", check histmap_Muon.txt, exiting.........."<<endl;
      exit(-1);
    }
    TList *histlist = file->GetListOfKeys();
    for(int i=0;i<histlist->Capacity();i++){
      TString this_cfname = histlist->At(i)->GetName();
      map_hist_Muon[a+"_"+this_cfname] = (TH1D *)file->Get(this_cfname);
      //cout << "[CFBackgroundEstimator::CFBackgroundEstimator] map_hist_Muon : " << a+"_"+this_cfname << endl;
    }
  }

}

CFBackgroundEstimator::~CFBackgroundEstimator(){

}

void CFBackgroundEstimator::SetDataYear(int i){
  DataYear = i;
}

double CFBackgroundEstimator::GetElectronCFRate(TString ID, TString key, double eta, double pt, int sys){

  //cout << "[CFBackgroundEstimator::GetElectronCFRate] ID = " << ID << ", key = " << key << endl;
  //cout << "[CFBackgroundEstimator::GetElectronCFRate] eta = " << eta << ", pt = " << pt << endl;

  double value = 1.;
  double error = 0.;

  eta = fabs(eta);
  if(eta>=2.5) eta = 2.49;

  TString EtaRegion = "InnerBarrel";
  if(eta<0.8) EtaRegion = "InnerBarrel";
  else if(eta<1.479) EtaRegion = "OuterBarrel";
  else EtaRegion = "EndCap";

  std::map< TString, TH1D* >::const_iterator mapit;
  mapit = map_hist_Electron.find(ID+"_"+key+"_"+EtaRegion+"_InvGenPt");

  if(mapit==map_hist_Electron.end()){
    cout << "[CFBackgroundEstimator::GetElectronCFRate] No"<< ID+"_"+key+"_"+EtaRegion+"_InvGenPt" <<endl;
    exit(EXIT_FAILURE);
  }

  int this_bin = (mapit->second)->FindBin(1./pt);
  value = (mapit->second)->GetBinContent(this_bin);
  error = (mapit->second)->GetBinError(this_bin);

  //cout << "[CFBackgroundEstimator::CFBackgroundEstimator] value = " << value << endl;

  return value+double(sys)*error;

}

double CFBackgroundEstimator::GetMuonCFRate(TString ID, TString key, double eta, double pt, int sys){

  //cout << "[CFBackgroundEstimator::GetMuonCFRate] ID = " << ID << ", key = " << key << endl;
  //cout << "[CFBackgroundEstimator::GetMuonCFRate] eta = " << eta << ", pt = " << pt << endl;

  double value = 1.;
  double error = 0.;

  eta = fabs(eta);

  if(eta>=2.5) eta = 2.49;

  TString EtaRegion = "InnerBarrel";
  if(eta<0.8) EtaRegion = "InnerBarrel";
  else if(eta<1.479) EtaRegion = "OuterBarrel";
  else EtaRegion = "EndCap";

  std::map< TString, TH1D* >::const_iterator mapit;
  mapit = map_hist_Muon.find(ID+"_"+key+"_"+EtaRegion+"_InvGenPt");

  if(mapit==map_hist_Muon.end()){
    cout << "[CFBackgroundEstimator::GetMuonCFRate] No"<< ID+"_"+key+"_"+EtaRegion+"_InvGenPt" <<endl;
    exit(EXIT_FAILURE);
  }

  int this_bin = (mapit->second)->FindBin(1./pt,eta);
  value = (mapit->second)->GetBinContent(this_bin);
  error = (mapit->second)->GetBinError(this_bin);

  //cout << "[CFBackgroundEstimator::CFBackgroundEstimator] value = " << value << endl;

  return value+double(sys)*error;

}

double CFBackgroundEstimator::GetWeight(vector<Lepton *> lepptrs, AnalyzerParameter param, int sys){

  double this_weight = 0.;
  for(unsigned int i=0; i<lepptrs.size(); i++){

    double this_cf = -999.;

    if(lepptrs.at(i)->LeptonFlavour()==Lepton::ELECTRON){

      Electron *el = (Electron *)( lepptrs.at(i) );

      this_cf = GetElectronCFRate(param.Electron_CF_ID, param.Electron_CF_Key, fabs(el->scEta()), el->Pt(), sys);

    }
    else{

      Muon *mu = (Muon *)( lepptrs.at(i) );

      this_cf = GetMuonCFRate(param.Muon_CF_ID, param.Muon_CF_Key, fabs(mu->Eta()), mu->Pt(), sys);

    }

    this_weight += this_cf/(1.-this_cf);

  }

  return this_weight;

}




