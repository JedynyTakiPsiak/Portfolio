#include <wx/wx.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sal.h>

class TicTacToeFrame : public wxFrame{
public:
    TicTacToeFrame(const wxString& title, const wxString& iconFile)
        : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(268, 468)),
        humanTurn(false), gameStarted(false)
    {
        SetIcon(wxIcon("logotest.ico", wxBITMAP_TYPE_ICO));
        Centre();

        wxPanel* panel = new wxPanel(this, wxID_ANY);

        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        resultText = new wxStaticText(panel, wxID_ANY, wxT("Wynik 0:0"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        mainSizer->Add(resultText, 0, wxALIGN_CENTER | wxALL, 10);

        currentTurnText = new wxStaticText(panel, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        mainSizer->Add(currentTurnText, 0, wxEXPAND | wxALL, 10);

        wxStaticBoxSizer* radioBoxSizer = new wxStaticBoxSizer(wxVERTICAL, panel, wxT("Poziom trudności"));
        easyRadioButton = new wxRadioButton(panel, wxID_ANY, wxT("łatwy"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
        mediumRadioButton = new wxRadioButton(panel, wxID_ANY, wxT("średni"));
        hardRadioButton = new wxRadioButton(panel, wxID_ANY, wxT("trudny"));

        radioBoxSizer->Add(easyRadioButton, 0, wxEXPAND | wxALL, 5);
        radioBoxSizer->Add(mediumRadioButton, 0, wxEXPAND | wxALL, 5);
        radioBoxSizer->Add(hardRadioButton, 0, wxEXPAND | wxALL, 5);

        mainSizer->Add(radioBoxSizer, 0, wxEXPAND | wxALL, 10);

        startButton = new wxButton(panel, wxID_ANY, wxT("start"));
        mainSizer->Add(startButton, 0, wxEXPAND | wxALL, 10);

        wxGridSizer* gridSizer = new wxGridSizer(3, 3, 0, 0);

        for (int i = 0; i < 9; ++i){
            wxButton* button = new wxButton(panel, 1000 + i, wxT(""));
            gridSizer->Add(button, 1, wxEXPAND | wxALL, 5);
            buttons.push_back(button);
            button->Bind(wxEVT_BUTTON, &TicTacToeFrame::OnGridButtonClick, this);
        }

        mainSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 10);

        panel->SetSizerAndFit(mainSizer);

        startButton->Bind(wxEVT_BUTTON, &TicTacToeFrame::OnStartButtonClick, this);

        std::srand(std::time(nullptr));
    }

private:
    std::vector<wxButton*> buttons;
    wxRadioButton* easyRadioButton;
    wxRadioButton* mediumRadioButton;
    wxRadioButton* hardRadioButton;
    wxStaticText* resultText;
    wxStaticText* currentTurnText;
    wxButton* startButton;
    wxString currentPlayer;
    wxString botPlayer;
    wxString humanPlayer;
    int humanScore = 0;
    int botScore = 0;
    bool humanTurn;
    bool gameStarted;

    void OnStartButtonClick(wxCommandEvent& event){
        ResetBoard();

        gameStarted = true;

        if (std::rand() % 2){
            humanTurn = true;
            currentPlayer = wxT("Twoja kolej");
            humanPlayer = wxT("X");
            botPlayer = wxT("O");
        }
        else{
            humanTurn = false;
            currentPlayer = wxT("Kolej przeciwnika");
            humanPlayer = wxT("O");
            botPlayer = wxT("X");
            if (easyRadioButton->GetValue())
                EasyBotMove();
            else if (mediumRadioButton->GetValue())
                MediumBotMove();
            else if (hardRadioButton->GetValue())
                HardBotMove();
        }

        UpdateButtonText();
    }

    void OnGridButtonClick(wxCommandEvent& event){
        if (!gameStarted)
            return;

        wxButton* clickedButton = static_cast<wxButton*>(event.GetEventObject());
        int index = std::distance(buttons.begin(), std::find(buttons.begin(), buttons.end(), clickedButton));

        if (clickedButton->GetLabel() == wxT("")){
            clickedButton->SetLabel(humanPlayer);
            clickedButton->SetForegroundColour(*wxBLACK);
            humanTurn = false;
            if (CheckWin(humanPlayer)){
                humanScore++;
                UpdateResultText();
                wxMessageBox(wxT("Wygrałeś!"));
                DisableButtons();
                gameStarted = false;
                return;
            }
            if (CheckDraw()){
                wxMessageBox(wxT("Remis!"));
                DisableButtons();
                gameStarted = false;
                return;
            }
            currentPlayer = wxT("Kolej przeciwnika");
            if (easyRadioButton->GetValue())
                EasyBotMove();
            else if (mediumRadioButton->GetValue())
                MediumBotMove();
            else if (hardRadioButton->GetValue())
                HardBotMove();
            if (CheckWin(botPlayer)){
                botScore++;
                UpdateResultText();
                wxMessageBox(wxT("Przeciwnik wygrał!"));
                DisableButtons();
                gameStarted = false;
                return;
            }
            if (CheckDraw()){
                wxMessageBox(wxT("Remis!"));
                DisableButtons();
                gameStarted = false;
                return;
            }
            currentPlayer = wxT("Twoja kolej");
        }
        UpdateButtonText();
    }

    void UpdateButtonText(){
        currentTurnText->SetLabelText(currentPlayer);
        for (int i = 0; i < 9; ++i){
            if (buttons[i]->GetLabel() == humanPlayer)
                buttons[i]->SetForegroundColour(*wxBLACK);
            else if (buttons[i]->GetLabel() == botPlayer)
                buttons[i]->SetForegroundColour(*wxRED);
        }

        Layout();
    }

    void UpdateResultText(){
        resultText->SetLabelText(wxString::Format(wxT("Wynik %d:%d"), humanScore, botScore));

        Layout();
    }

    bool CheckWin(const wxString& player){
        for (int i = 0; i < 3; ++i){
            if (buttons[i]->GetLabel() == player && buttons[i + 3]->GetLabel() == player && buttons[i + 6]->GetLabel() == player)
                return true;
            if (buttons[3 * i]->GetLabel() == player && buttons[3 * i + 1]->GetLabel() == player && buttons[3 * i + 2]->GetLabel() == player)
                return true;
        }
        if (buttons[0]->GetLabel() == player && buttons[4]->GetLabel() == player && buttons[8]->GetLabel() == player)
            return true;
        if (buttons[2]->GetLabel() == player && buttons[4]->GetLabel() == player && buttons[6]->GetLabel() == player)
            return true;
        return false;
    }

    bool CheckDraw(){
        for (int i = 0; i < 9; ++i){
            if (buttons[i]->GetLabel() == wxT(""))
                return false;
        }
        return true;
    }

    void DisableButtons(){
        for (auto button : buttons){
            button->Disable();
        }
    }

    void ResetBoard(){
        for (auto button : buttons){
            button->SetLabel(wxT(""));
            button->Enable();
        }
        currentPlayer = wxT("");
        currentTurnText->SetLabel(currentPlayer);
        gameStarted = false;
        humanTurn = false;
    }

    void EasyBotMove(){
        if (!gameStarted) return;

        int index;
        do{
            index = std::rand() % 9;
        } while (buttons[index]->GetLabel() != wxT(""));
        buttons[index]->SetLabel(botPlayer);
        buttons[index]->SetForegroundColour(*wxRED);
        humanTurn = true;
    }

    void MediumBotMove(){
        if (!gameStarted) return;

        if (MakeWinningMove(botPlayer))
            return;
        if (MakeWinningMove(humanPlayer))
            return;
        EasyBotMove();
    }

    void HardBotMove(){
        if (!gameStarted) return;

        if (MakeWinningMove(botPlayer))
            return;
        if (MakeWinningMove(humanPlayer))
            return;
        if (buttons[4]->GetLabel() == wxT("")){
            buttons[4]->SetLabel(botPlayer);
            buttons[4]->SetForegroundColour(*wxRED);
            humanTurn = true;
            return;
        }
        for (int i : { 0, 2, 6, 8 }){
            if (buttons[i]->GetLabel() == wxT("")){
                buttons[i]->SetLabel(botPlayer);
                buttons[i]->SetForegroundColour(*wxRED);
                humanTurn = true;
                return;
            }
        }
        EasyBotMove();
    }

    bool MakeWinningMove(const wxString& player){
        for (int i = 0; i < 3; ++i){
            if (buttons[i]->GetLabel() == player && buttons[i + 3]->GetLabel() == player && buttons[i + 6]->GetLabel() == wxT("")){
                buttons[i + 6]->SetLabel(botPlayer);
                buttons[i + 6]->SetForegroundColour(*wxRED);
                humanTurn = true;
                return true;
            }
            if (buttons[3 * i]->GetLabel() == player && buttons[3 * i + 1]->GetLabel() == player && buttons[3 * i + 2]->GetLabel() == wxT("")){
                buttons[3 * i + 2]->SetLabel(botPlayer);
                buttons[3 * i + 2]->SetForegroundColour(*wxRED);
                humanTurn = true;
                return true;
            }
        }
        if (buttons[0]->GetLabel() == player && buttons[4]->GetLabel() == player && buttons[8]->GetLabel() == wxT("")){
            buttons[8]->SetLabel(botPlayer);
            buttons[8]->SetForegroundColour(*wxRED);
            humanTurn = true;
            return true;
        }
        if (buttons[2]->GetLabel() == player && buttons[4]->GetLabel() == player && buttons[6]->GetLabel() == wxT("")){
            buttons[6]->SetLabel(botPlayer);
            buttons[6]->SetForegroundColour(*wxRED);
            humanTurn = true;
            return true;
        }
        return false;
    }
};

class TicTacToeApp : public wxApp{
public:
    virtual bool OnInit(){
        TicTacToeFrame* frame = new TicTacToeFrame(wxT("Tic Tac Toe"), wxT("logotest.ico"));
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(TicTacToeApp);
