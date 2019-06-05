#include <TCanvas.h>
#include <TMultiGraph.h>
#include <TColor.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <TLegend.h>
#include <TAxis.h>
#include <TRint.h>
#include <TSystem.h>
#include <TROOT.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <cmath>

using namespace std;

struct graphinfo{
  TGraphErrors* gr;
  TF1* func;
  double* x;
  double* ex;
  double* y;
  double* ey;
  string title = "noname";
  string key = "nokey";
  string xaxis = "xaxis";
  string yaxis = "yaxis";
  int len;
  int color = kBlack;
  bool toFit = false;
  bool logx = false;
  bool logy = false;
};

void printHelp()
{
  cout << "\x1b[0m";
  cout << "Multiplotter v1.0 - By Eduardo Ferreira" << endl;
  cout << "Multiplotter is a tool to plot several graphics simultaneosly using CERN/ROOT" << endl;
  cout << "\nUsage: multiplotter [arguments] --input <filename>" << endl;
  cout << " --input - input filename - this file must follow multiplotter syntax (see example)" << endl;
  cout << " --output - output filename" << endl;
  cout << " --nogui - no gui, for batch programs" << endl;
  cout << " --separate - does the plots in different files" << endl;
}


double fitfunc(double* x, double* par)
{
  return par[0]*(1 - 1/sqrt(1 + pow(par[1]/(par[2] + fabs(x[0])), 2)));
}

int main(int argc, char** argv)
{
  string filename = "";
  string outfilename = "";
  bool nogui = false;
  bool separate = false;

  cout << "Hello!  \x1b[1;32mMultiplotter\x1b[0m - Version 1.0 " << endl;

  if(argc == 1)
  {
    printHelp();
    return 1;
  }

  bool fail = false;
  for (int i = 1; i<argc; i++)
  {
    string argument = string(argv[i]);

    if(argument == "--input")
    {
      if(i + 1 < argc)
      {
        filename = argv[i+1];
        if(filename.find("--", 0) == string::npos)
        {
          i++;
          continue;
        }
        else
          fail = true;
      }
      else {fail = true;}
    }
    else if(argument == "--output")
    {
      if(i + 1 < argc)
      {
        outfilename = argv[i+1];
        if(outfilename.find("--", 0) == string::npos)
        {
          i++;
          continue;
        }
        else
          fail = true;
      }
      else {fail = true;}
    }
    else if(argument == "--nogui")
    {
      nogui = true;
      continue;
    }
    else if(argument == "--separate")
    {
      separate = true;
      continue;
    }
    else
      fail = true;
  }

  if(fail)
  {
    printHelp();
    return 1;
  }

  ifstream* filein = new ifstream(filename.c_str());
  if(filein->fail())
  {
    cout << "\x1b[1;31mError:\x1b[0m Failure opening file." << endl;
    filein->close();
    delete filein;
    return 1;
  }

  TRint *theApp = new TRint("Multiplotter", &argc, argv);
  TCanvas* tc = new TCanvas();
  TMultiGraph* mg = new TMultiGraph();
  TLegend *tg = new TLegend();

  vector<graphinfo> big_list;

  int nlist = 0;

  int i = 0;
  bool inGraph = false;
  while(!filein->eof())
  {
    string line;
    getline(*filein, line);
    //cout << "Line: " << line << endl;
    istringstream parser(line);
    if(line.find("#") != string::npos)
    {
      //nlist++;
      //i = 0;
      //If this is a title, x or y axis name. We need to do things differently
      //This is a configuration line
      vector<string> tokens ((istream_iterator<string>(parser)), istream_iterator<string>());
      for(vector<string>::iterator it = tokens.begin(); it != tokens.end(); it++)
      {
        if (it->compare("BeginGraph") == 0)
        {
          if(!inGraph)
          {
            nlist++;
            i = 0;
            big_list.push_back(graphinfo());
            inGraph = true;
          }
          else
          {
            cout << "There is a problem with your configuration file" << endl;
            cout << "Multiplotter cannot safely continue. Exiting!" << endl;
            return 1;
          }
        }
        if (it->compare("EndGraph") == 0)
        {
          if(inGraph)
          {
            inGraph = false;
          }
          else
          {
            cout << "There is a problem with your configuration file" << endl;
            cout << "Multiplotter cannot safely continue. Exiting!" << endl;
            return 1;
          }
        }
        else if (it->compare("Length:") == 0)
        {
          if(inGraph){
            string op = *(it+1);
            try{
              if(big_list[nlist-1].len == 0 && stoi(op) != 0)
              {
                big_list[nlist-1].len = stoi(op);
                big_list[nlist-1].x = new double[big_list[nlist-1].len];
                big_list[nlist-1].y = new double[big_list[nlist-1].len];
                big_list[nlist-1].ex = new double[big_list[nlist-1].len];
                big_list[nlist-1].ey = new double[big_list[nlist-1].len];
              }
            } catch (invalid_argument e)
            {
              cout << "ERROR: Malformed Length directive" << endl;
              big_list[nlist-1].len = 100;
            }
          }
          else
          {
            cout << "Length directive outside of graph. Ignoring!" << endl;
          }
        }
        else if (it->compare("SetLogX") == 0)
        {
          if(inGraph)
          {
            big_list[nlist-1].logx = true;
          }
          else
          {
            cout << "ERROR: Malformed Key Directive" << endl;
          }
        }
        else if (it->compare("SetLogY") == 0)
        {
          if(inGraph)
          {
            big_list[nlist-1].logy = true;
          }
          else
          {
            cout << "ERROR: Malformed Key Directive" << endl;
          }
        }
        else if (it->compare("Key:") == 0)
        {
          if(inGraph){
            string op = *(it+1);
            if (op.compare("") == 0)
            {
              cout << "ERROR: Malformed Key Directive" << endl;
              big_list[nlist-1].key = "Graph" + to_string(nlist-1);
            }else{
              big_list[nlist-1].key = op;
            }
          }
          else
          {
            cout << "Key directive outside of graph. Ignoring!" << endl;
          }
        }
        else if (it->compare("Color:") == 0)
        {
          if(inGraph){
            string op = *(it+1);
            try{
              big_list[nlist-1].color = stoi(op);
            } catch (invalid_argument e)
            {
              cout << "ERROR: Malformed Color directive" << endl;
              big_list[nlist-1].color = kBlack;
            }
          }
          else
          {
            cout << "Color directive outside of graph. Ignoring!" << endl;
          }
        }
        else if(it->compare("Fit:") == 0)
        {
          if(inGraph)
          {
            string op = *(it+1);
            if(op == "1")
              big_list[nlist-1].toFit = 1;
            else if(op == "0")
              big_list[nlist-1].toFit = 0;
            else
            {
              cout << "ERROR: Malformed Fit directive" << endl;
            }
          }
          else
          {
            cout << "Fit directive outside of graph. Ignoring!" << endl;
          }
        }
        else if(it->compare("XAxis:") == 0)
        {
          if(inGraph)
          {
            int index = line.find("# XAxis: ", 0);
            if(index != string::npos)
            {
              big_list[nlist-1].xaxis = string(line, index+9);
            }
            else
            {
              //this requires more formal syntax
              cout << "ERROR: Malformed XAxis directive" << endl;
            }
          }
          else
          {
            cout << "XAxis directive outside of graph. Ignoring!" << endl;
          }
        }
        else if(it->compare("YAxis:") == 0)
        {
          if(inGraph)
          {
            int index = line.find("# YAxis: ", 0);
            if(index != string::npos)
            {
              big_list[nlist-1].yaxis = string(line, index+9);
            }
            else
            {
              //this requires more formal syntax
              cout << "ERROR: Malformed YAxis directive" << endl;
            }
          }
          else
          {
            cout << "YAxis directive outside of graph. Ignoring!" << endl;
          }
        }
        else if(it->compare("Title:") == 0)
        {
          if(inGraph)
          {
            int index = line.find("# Title: ", 0);
            if(index != string::npos)
            {
              big_list[nlist-1].title = string(line, index+9);
            }
            else
            {
              //this requires more formal syntax
              cout << "ERROR: Malformed Title directive" << endl;
            }
          }
          else
          {
            cout << "Title directive outside of graph. Ignoring!" << endl;
          }
        }

      }
    } else {
      if(inGraph)
      {
        istringstream(line) >> big_list[nlist-1].x[i] >> big_list[nlist-1].ex[i] >> big_list[nlist-1].y[i] >> big_list[nlist-1].ey[i];
        i++;
      } else {
        cout << "There is data outside of a graph. You might want to check your limits." << endl;
      }
    }
  }

  int j = 0;
  for(graphinfo& gi : big_list)
  {
    gi.gr = new TGraphErrors(gi.len, gi.x, gi.y, gi.ex, gi.ey);
    gi.gr->SetTitle(gi.title.c_str());
    gi.gr->SetLineColor(kBlack);
    if(gi.toFit)
    {
      //gi.func = new TF1((string("f") + to_string(j)).c_str(), fitfunc, -1000, 1000, 3);
      //Custom parameters:
      //gi.func->FixParameter(1, 2.5);
      //gi.func->SetParameter(2, 15.8);
      //gi.func->FixParameter(3, 1 - 2*j);
      // lalalalalalalalalal
      //gi.func->SetLineColor(gi.color);
      //TFitResultPtr ptr = gi.gr->Fit(gi.func, "S");
      //ptr->Print("V");
    }
    else
    {
      gi.gr->SetLineColor(gi.color);
    }
    if(!separate)
    {
      mg->Add(gi.gr);
      tg->AddEntry(gi.func, gi.key.c_str(), "l");
    }
    else
    {
      TCanvas* tc2 = new TCanvas();
      tc2->cd();
      tc2->SetLogx(gi.logx);
      tc2->SetLogy(gi.logy);
      gi.gr->Draw("AP");
      gi.gr->GetXaxis()->SetTitle(gi.xaxis.c_str());
      gi.gr->GetYaxis()->SetTitle(gi.yaxis.c_str());
      //tc2->Update();
      tc2->SaveAs((outfilename + to_string(j) + ".png").c_str());
      tc2->SaveAs((outfilename + to_string(j) + ".eps").c_str());
      delete tc2;
    }
    j++;
  }

  if(!separate)
  {
    tc->SetLogx(big_list[0].logx);
    tc->SetLogy(big_list[0].logy);

    mg->SetTitle("Dist#hat{a}ncia ao centro no eixo dos detetores");
    mg->Draw("AP");
    mg->GetXaxis()->SetTitle("y [cm]");
    mg->GetYaxis()->SetTitle("Rate [cts/s]");

    tg->SetNColumns(2);
    //tg->SetLineWidth(10);
    tg->Draw();
    tc->Update();
    //tc->WaitPrimitive();

    tc->SaveAs((outfilename + ".png").c_str());
    tc->SaveAs((outfilename + ".eps").c_str());
  }
  if(!nogui && !separate)
  {
    theApp->Run();
  }

  for(graphinfo g : big_list)
  {
    delete g.func;
    delete g.gr;
    delete[] g.ex;
    delete[] g.ey;
    delete[] g.y;
    delete[] g.x;
  }

  delete tg;
  delete mg;
  delete tc;
  delete theApp;
  filein->close();
  delete filein;

  return 0;
}
