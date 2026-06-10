// Jakub Ćwiek 16.12.2024r.
// 19. Gaz sieciowy 4-gatunkowy. Konfiguracja początkowa losowa (suwaki lub pola tekstowe).
// Bias (suwak) to preferencja w kierunku: lewo(A), prawo(B), góra(C), dół(D). 
// Uwzględnienie klasteryzacji: jeżeli cząstka ma przynajmniej 2 sąsiadów tego samego rodzaju co ona to pozostaje nieruchoma. 

import java.awt.*;
import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.event.*;

public class Main {
    public static void main(String[] args) {
        System.out.println("Kodowanie JVM: " + System.getProperty("file.encoding"));
        System.out.println("Lokalizacja JVM: " + System.getProperty("user.language"));
        JFrame frame = new JFrame("Projekt nr 19 Jakub Ćwiek");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setSize(1111, 666);
        frame.setLocationRelativeTo(null);
        
        // Czcionka
        Font customFont = new Font("Arial", Font.BOLD, 18);
        
        // Główny panel
        JPanel mainPanel = new JPanel(new BorderLayout());
        mainPanel.setBackground(Color.BLACK);
        
        // Panel na wykres (po lewej stronie)
        JPanel leftPanel = new JPanel(new BorderLayout());
        leftPanel.setBackground(Color.BLACK);
        Wykres wykres = new Wykres();
        wykres.setPreferredSize(new Dimension(444, 444));
        //leftPanel.add(wykres, new GridBagConstraints());  // Nie łączy się z paskiem narzedzi
        //mainPanel.add(leftPanel, BorderLayout.CENTER);

        // Panel na narzedzia
        JPanel toolsPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 10, 0));
        toolsPanel.setBackground(Color.BLACK);
        
        // Tworzenie kolorów
        Color[] colors = {Color.red, Color.blue, Color.yellow, Color.green, Color.white};
        for (int i = 0; i < colors.length; i++) {
            Color color = colors[i];
            int liczba = i + 1;
        
            JButton colorButton = new JButton();
            colorButton.setBackground(color);
            colorButton.setPreferredSize(new Dimension(20, 20));
            colorButton.setBorder(BorderFactory.createLineBorder(Color.BLACK));
        
            colorButton.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent arg0) {
                    wykres.kolor = liczba;
                }
            });
        
            toolsPanel.add(colorButton);
        }

        JButton olowekButton = new JButton("Ołówek");
        olowekButton.setFont(customFont);
        olowekButton.setForeground(Color.WHITE);
        olowekButton.setBackground(Color.BLACK);
        olowekButton.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3),
            BorderFactory.createEmptyBorder(5, 5, 5, 5)
        ));
        olowekButton.setAlignmentX(Component.CENTER_ALIGNMENT);
        olowekButton.setMaximumSize(new Dimension(Integer.MAX_VALUE, olowekButton.getPreferredSize().height));
        olowekButton.setPreferredSize(new Dimension(111, 30));
        olowekButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                wykres.narzedzie = "olowek";
            }
        });
        toolsPanel.add(olowekButton);

        JButton gumkaButton = new JButton("Gumka");
        gumkaButton.setFont(customFont);
        gumkaButton.setForeground(Color.WHITE);
        gumkaButton.setBackground(Color.BLACK);
        gumkaButton.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3),
            BorderFactory.createEmptyBorder(5, 5, 5, 5)
        ));
        gumkaButton.setAlignmentX(Component.CENTER_ALIGNMENT);
        gumkaButton.setMaximumSize(new Dimension(Integer.MAX_VALUE, gumkaButton.getPreferredSize().height));
        gumkaButton.setPreferredSize(new Dimension(111, 30));
        gumkaButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                wykres.narzedzie = "gumka";
            }
        });
        toolsPanel.add(gumkaButton);

        toolsPanel.setBorder(BorderFactory.createEmptyBorder(50, 0, 20, 0));
        leftPanel.add(toolsPanel, BorderLayout.NORTH);
        JPanel wykresWrapper = new JPanel(new FlowLayout(FlowLayout.CENTER, 0, 0)); // Rozwiązanie z StackOverFlow aby dodać wrapper
        wykresWrapper.setBackground(Color.BLACK);
        wykresWrapper.add(wykres);
        leftPanel.add(wykresWrapper, BorderLayout.CENTER);
        mainPanel.add(leftPanel, BorderLayout.CENTER);

        // Panel z suwakami (po prawej stronie)
        JPanel controlPanel = new JPanel();
        controlPanel.setLayout(new BoxLayout(controlPanel, BoxLayout.Y_AXIS));
        controlPanel.setBorder(BorderFactory.createEmptyBorder(20, 10, 20, 10));
        controlPanel.setBackground(Color.BLACK);
        controlPanel.setPreferredSize(new Dimension(333, 0));
        JLabel label_suwak1 = new JLabel("",SwingConstants.CENTER);
        JLabel label_suwak2 = new JLabel("",SwingConstants.CENTER);
        JLabel label_suwak3 = new JLabel("",SwingConstants.CENTER);
        JLabel label_suwak_density = new JLabel("",SwingConstants.CENTER);
        label_suwak1.setFont(customFont);
        label_suwak1.setForeground(Color.WHITE);
        label_suwak1.setAlignmentX(Component.CENTER_ALIGNMENT);
        label_suwak2.setFont(customFont);
        label_suwak2.setForeground(Color.WHITE);
        label_suwak2.setAlignmentX(Component.CENTER_ALIGNMENT);
        label_suwak3.setFont(customFont);
        label_suwak3.setForeground(Color.WHITE);
        label_suwak3.setAlignmentX(Component.CENTER_ALIGNMENT);
        label_suwak_density.setFont(customFont);
        label_suwak_density.setForeground(Color.WHITE);
        label_suwak_density.setAlignmentX(Component.CENTER_ALIGNMENT);

        // Guzik START/RESET
        JButton button = new JButton("START");
        JCheckBox checkbox1 = new JCheckBox("");
        JCheckBox checkbox2 = new JCheckBox("");
        JCheckBox checkbox3 = new JCheckBox("");
        JCheckBox checkbox4 = new JCheckBox("");  // Tworzymy wcześniej aby można byłoby naprawiać błąd logiczny przy RESET

        button.setFont(new Font("Arial", Font.BOLD, 28));
        button.setForeground(Color.WHITE);
        button.setBackground(Color.BLACK);
        button.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3), // Zewnętrzna ramka
            BorderFactory.createEmptyBorder(10, 20, 10, 20)  // Wewnętrzny margin
        ));
        button.setAlignmentX(Component.CENTER_ALIGNMENT);
        button.setMaximumSize(new Dimension(Integer.MAX_VALUE, button.getPreferredSize().height)); // Coś aby wyśrodkować guzik
        button.setFocusPainted(false);  // Aby usunąć obramowanie podczas wybrania elementu
        button.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                wykres.rysowac = true;
                wykres.initial();
                wykres.repaint();
                button.setText("RESET");
                checkbox1.setSelected(true);
                checkbox2.setSelected(true);
                checkbox3.setSelected(true);
                checkbox4.setSelected(true);
            }
        });
        controlPanel.add(button);
        controlPanel.add(Box.createRigidArea(new Dimension(0, 20)));  // Sztuczny odstęp między guzikami

        // Panel na checkbox'y
        JPanel checkboxPanel = new JPanel();
        checkboxPanel.setLayout(new FlowLayout(FlowLayout.CENTER, 30, 0));
        checkboxPanel.setBackground(Color.BLACK);
        checkboxPanel.setPreferredSize(new Dimension(0, 50));

        checkbox1.setBackground(Color.RED);
        checkbox1.setForeground(Color.WHITE);
        checkbox1.setFocusPainted(false);
        checkbox1.setSelected(true);
        checkbox1.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                boolean isChecked = checkbox1.isSelected();
                wykres.turn_off_on(1, isChecked);
                wykres.repaint();
            }
        });

        checkbox2.setBackground(Color.BLUE);
        checkbox2.setForeground(Color.WHITE);
        checkbox2.setFocusPainted(false);
        checkbox2.setSelected(true);
        checkbox2.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                boolean isChecked = checkbox2.isSelected();
                wykres.turn_off_on(2, isChecked);
                wykres.repaint();
            }
        });

        checkbox3.setBackground(Color.YELLOW);
        checkbox3.setForeground(Color.BLACK);
        checkbox3.setFocusPainted(false);
        checkbox3.setSelected(true);
        checkbox3.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                boolean isChecked = checkbox3.isSelected();
                wykres.turn_off_on(3, isChecked);
                wykres.repaint();
            }
        });

        checkbox4.setBackground(Color.GREEN);
        checkbox4.setForeground(Color.WHITE);
        checkbox4.setFocusPainted(false);
        checkbox4.setSelected(true);
        checkbox4.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                boolean isChecked = checkbox4.isSelected();
                wykres.turn_off_on(4, isChecked);
                wykres.repaint();
            }
        });

        checkboxPanel.add(checkbox1);
        checkboxPanel.add(checkbox2);
        checkboxPanel.add(checkbox3);
        checkboxPanel.add(checkbox4);

        JPanel opisPanel = new JPanel();
        opisPanel.setBackground(Color.BLACK);
        opisPanel.setLayout(new FlowLayout(FlowLayout.CENTER));

        JLabel opisLabel = new JLabel("<html>Włącz/Wyłącz wybrany gaz<br>gęstość cząstek w %</html>", SwingConstants.CENTER);
        opisLabel.setFont(customFont);
        opisLabel.setForeground(Color.WHITE);

        opisPanel.add(opisLabel);

        controlPanel.add(opisPanel);
        controlPanel.add(Box.createRigidArea(new Dimension(0, 10)));
        controlPanel.add(checkboxPanel);

        // Panel na suwaki gęstości cząstek
        JPanel densityPanel = new JPanel();
        densityPanel.setLayout(new GridLayout(1, 4, 5, 10));
        densityPanel.setBackground(Color.BLACK);
        densityPanel.setPreferredSize(new Dimension(0, 150));

        // Gęstość cząstek suwaki
        Dimension sliderSize = new Dimension(50, 100); 
        JSlider suwak_density1 = new JSlider(JSlider.VERTICAL, 0, 100, 100);
        suwak_density1.setMajorTickSpacing(25);
        suwak_density1.setPaintTicks(true);
        suwak_density1.setPaintLabels(true);
        suwak_density1.setBackground(Color.BLACK);
        suwak_density1.setForeground(Color.WHITE);
        suwak_density1.setPreferredSize(sliderSize);
        suwak_density1.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                wykres.density1 = suwak_density1.getValue() / 100.0;
                wykres.repaint();
            }
        });
        densityPanel.add(suwak_density1);
        
        JSlider suwak_density2 = new JSlider(JSlider.VERTICAL, 0, 100, 100);
        suwak_density2.setMajorTickSpacing(25);
        suwak_density2.setPaintTicks(true);
        suwak_density2.setPaintLabels(true);
        suwak_density2.setBackground(Color.BLACK);
        suwak_density2.setForeground(Color.WHITE);
        suwak_density2.setPreferredSize(sliderSize);
        suwak_density2.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                wykres.density2 = suwak_density2.getValue() / 100.0;
                wykres.repaint();
            }
        });
        densityPanel.add(suwak_density2);
        
        JSlider suwak_density3 = new JSlider(JSlider.VERTICAL, 0, 100, 100);
        suwak_density3.setMajorTickSpacing(25);
        suwak_density3.setPaintTicks(true);
        suwak_density3.setPaintLabels(true);
        suwak_density3.setBackground(Color.BLACK);
        suwak_density3.setForeground(Color.WHITE);
        suwak_density3.setPreferredSize(sliderSize);
        suwak_density3.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                wykres.density3 = suwak_density3.getValue() / 100.0;
                wykres.repaint();
            }
        });
        densityPanel.add(suwak_density3);
        
        JSlider suwak_density4 = new JSlider(JSlider.VERTICAL, 0, 100, 100);
        suwak_density4.setMajorTickSpacing(25);
        suwak_density4.setPaintTicks(true);
        suwak_density4.setPaintLabels(true);
        suwak_density4.setBackground(Color.BLACK);
        suwak_density4.setForeground(Color.WHITE);
        suwak_density4.setPreferredSize(sliderSize);
        suwak_density4.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent e) {
                wykres.density4 = suwak_density4.getValue() / 100.0;
                wykres.repaint();
            }
        });
        densityPanel.add(suwak_density4);
        
        controlPanel.add(densityPanel);
        controlPanel.add(Box.createRigidArea(new Dimension(0, 20)));

       // Suwaki i ich etykiety
       JSlider suwak1 = new JSlider(JSlider.HORIZONTAL, 0, 20, 1);
       suwak1.addChangeListener(new ChangeListener() {
           public void stateChanged(ChangeEvent e) {
               wykres.speed = suwak1.getValue();
               wykres.repaint();
               label_suwak1.setText("speed = " + suwak1.getValue());
           }
       });
       label_suwak1.setText("speed = " + suwak1.getValue());
       controlPanel.add(label_suwak1);
       controlPanel.add(suwak1);
       controlPanel.add(Box.createRigidArea(new Dimension(0, 20)));

       JSlider suwak2 = new JSlider(JSlider.HORIZONTAL, -25, 25, 0);
       suwak2.addChangeListener(new ChangeListener() {
           public void stateChanged(ChangeEvent e) {
               wykres.bias_poziom = 0.01 * suwak2.getValue();
               wykres.repaint();
               label_suwak2.setText("bias poziomy = " + 0.01 * suwak2.getValue());
           }
       });
       label_suwak2.setText("bias poziomy = " + 0.01 * suwak2.getValue());
       controlPanel.add(label_suwak2);
       controlPanel.add(suwak2);
       controlPanel.add(Box.createRigidArea(new Dimension(0, 20)));

       JSlider suwak3 = new JSlider(JSlider.HORIZONTAL, -25, 25, 0);
       suwak3.addChangeListener(new ChangeListener() {
           public void stateChanged(ChangeEvent e) {
               wykres.bias_pion = 0.01 * suwak3.getValue();
               wykres.repaint();
               label_suwak3.setText("bias pionowy = " + 0.01 * suwak3.getValue());
           }
       });
       label_suwak3.setText("bias pionowy = " + 0.01 * suwak3.getValue());
       controlPanel.add(label_suwak3);
       controlPanel.add(suwak3);
       controlPanel.add(Box.createRigidArea(new Dimension(0, 100)));  // Sztuczny odstęp między guzikami

        // Panel z guzikami (po prawej stronie)
        JPanel rightPanel = new JPanel();
        rightPanel.setLayout(new BoxLayout(rightPanel, BoxLayout.Y_AXIS));
        rightPanel.setBorder(BorderFactory.createEmptyBorder(20, 10, 20, 10));
        rightPanel.setBackground(Color.BLACK);
        rightPanel.setPreferredSize(new Dimension(268, 0));

        // Guzik Klasteryzacja
        JButton buttonK = new JButton("<html>Klasteryzacja<br>Włączona</html>");  // html trik z nową linią
        buttonK.setFont(customFont);
        buttonK.setForeground(Color.WHITE);
        buttonK.setBackground(Color.BLACK);
        buttonK.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3), // Zewnętrzna ramka
            BorderFactory.createEmptyBorder(10, 20, 10, 20)  // Wewnętrzny margin
        ));
        buttonK.setAlignmentX(Component.CENTER_ALIGNMENT);
        buttonK.setMaximumSize(new Dimension(Integer.MAX_VALUE, buttonK.getPreferredSize().height)); // Coś aby wyśrodkować guzik
        buttonK.setFocusPainted(false);  // Aby usunąć obramowanie podczas wybrania elementu
        buttonK.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                if (!wykres.klasteryzacja) {
                    wykres.klasteryzacja = true;
                    buttonK.setText("<html>Klasteryzacja<br>Włączona</html>");
                } else {
                    wykres.klasteryzacja = false;
                    buttonK.setText("<html>Klasteryzacja<br>Wyłączona</html>");
                }
            }
        });
        rightPanel.add(buttonK);
        rightPanel.add(Box.createRigidArea(new Dimension(0, 20)));  // Sztuczny odstęp między guzikami

        // Guzik Anihilacja AA
        JButton buttonAA = new JButton("<html>Anihilacja AA<br>Wyłączona</html>");  // html trik z nową linią
        buttonAA.setFont(customFont);
        buttonAA.setForeground(Color.WHITE);
        buttonAA.setBackground(Color.BLACK);
        buttonAA.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3), // Zewnętrzna ramka
            BorderFactory.createEmptyBorder(10, 20, 10, 20)  // Wewnętrzny margin
        ));
        buttonAA.setAlignmentX(Component.CENTER_ALIGNMENT);
        buttonAA.setMaximumSize(new Dimension(Integer.MAX_VALUE, buttonAA.getPreferredSize().height)); // Coś aby wyśrodkować guzik
        buttonAA.setFocusPainted(false);  // Aby usunąć obramowanie podczas wybrania elementu
        buttonAA.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                if (!wykres.anihilacjaAA) {
                    wykres.anihilacjaAA = true;
                    buttonAA.setText("<html>Anihilacja AA<br>Włączona</html>");
                } else {
                    wykres.anihilacjaAA = false;
                    buttonAA.setText("<html>Anihilacja AA<br>Wyłączona</html>");
                }
            }
        });
        rightPanel.add(buttonAA);
        rightPanel.add(Box.createRigidArea(new Dimension(0, 20)));  // Sztuczny odstęp między guzikami

        // Guzik Anihilacja AB
        JButton buttonAB = new JButton("<html>Anihilacja AB<br>Wyłączona</html>");  // html trik z nową linią
        buttonAB.setFont(customFont);
        buttonAB.setForeground(Color.WHITE);
        buttonAB.setBackground(Color.BLACK);
        buttonAB.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3), // Zewnętrzna ramka
            BorderFactory.createEmptyBorder(10, 20, 10, 20)  // Wewnętrzny margin
        ));
        buttonAB.setAlignmentX(Component.CENTER_ALIGNMENT);
        buttonAB.setMaximumSize(new Dimension(Integer.MAX_VALUE, buttonAB.getPreferredSize().height)); // Coś aby wyśrodkować guzik
        buttonAB.setFocusPainted(false);  // Aby usunąć obramowanie podczas wybrania elementu
        buttonAB.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent arg0) {
                if (!wykres.anihilacjaAB) {
                    wykres.anihilacjaAB = true;
                    buttonAB.setText("<html>Anihilacja AB<br>Włączona</html>");
                } else {
                    wykres.anihilacjaAB = false;
                    buttonAB.setText("<html>Anihilacja AB<br>Wyłączona</html>");
                }
            }
        });
        rightPanel.add(buttonAB);
        rightPanel.add(Box.createRigidArea(new Dimension(0, 20)));  // Sztuczny odstęp między guzikami

        // Wybór która cząstka anihiluje się z którą jeśli jest włączona Anihilacja AB
        JPanel anihilacjaPanel = new JPanel();
        anihilacjaPanel.setLayout(new BoxLayout(anihilacjaPanel, BoxLayout.Y_AXIS));
        anihilacjaPanel.setBackground(Color.BLACK);
        anihilacjaPanel.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Color.WHITE, 3), // Zewnętrzna ramka
            BorderFactory.createEmptyBorder(10, 20, 10, 20)  // Wewnętrzny margin
        ));

        JLabel anihilacjaLabel = new JLabel("<html>Reakcja cząstek<br>między sobą</html>", SwingConstants.CENTER);
        anihilacjaLabel.setFont(customFont);
        anihilacjaLabel.setForeground(Color.WHITE);
        anihilacjaLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        anihilacjaPanel.add(anihilacjaLabel);
        anihilacjaPanel.add(Box.createRigidArea(new Dimension(0, 10))); 

        String[] opcje = {"A", "B", "C", "D"};
        String[] litery = {"A", "B", "C", "D"};
        for (String litera : litery) {  // Rozwiązanie pomocne z StackOverFlow
            JPanel opcjaPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
            opcjaPanel.setBackground(Color.BLACK);

            // Etykieta dla danej opcji
            JLabel literaLabel = new JLabel(litera + ":");
            literaLabel.setFont(customFont);
            literaLabel.setForeground(Color.WHITE);

            // Combobox do wyboru opcji
            JComboBox<String> comboBox = new JComboBox<>(opcje);
            comboBox.setFont(customFont);
            comboBox.setBackground(Color.WHITE);
            comboBox.setForeground(Color.BLACK);
            comboBox.setFocusable(false);
            comboBox.setPreferredSize(new Dimension(80, 25));
            switch (litera) {
                case "A":
                    comboBox.setSelectedItem("B");
                    break;
                case "B":
                    comboBox.setSelectedItem("C");
                    break;
                case "C":
                    comboBox.setSelectedItem("D");
                    break;
                case "D":
                    comboBox.setSelectedItem("A");
                    break;
                default:
                    comboBox.setSelectedItem("A");
                    break;
            }
            comboBox.addActionListener(new ActionListener() {
                @Override
                public void actionPerformed(ActionEvent e) {
                    int index = java.util.Arrays.asList(litery).indexOf(litera);
                    if (index == -1) {  // Sprawdzenie wyjścia poza zakres
                        throw new IllegalArgumentException("Nie znaleziono litery: " + litera);
                    }
                    wykres.anihilacjaAB_nowy[index] = comboBox.getSelectedIndex() + 1;
                    // System.out.println("Wybrano " + index + ": " + wykres.anihilacjaAB_nowy[index]);
                    // System.out.println(wykres.anihilacjaAB_nowy[0] + " " + wykres.anihilacjaAB_nowy[1] + " " + wykres.anihilacjaAB_nowy[2] + " "  + wykres.anihilacjaAB_nowy[3]);
                }
            });

            opcjaPanel.add(literaLabel);
            opcjaPanel.add(comboBox);

            anihilacjaPanel.add(opcjaPanel);
        }   
        rightPanel.add(anihilacjaPanel);

        mainPanel.add(controlPanel, BorderLayout.WEST);
        mainPanel.add(rightPanel, BorderLayout.EAST);
        frame.add(mainPanel);
        frame.setVisible(true);
    }
}

// lewo(A), prawo(B), góra(C), dół(D). 
// lattice = 0 => puste
// latticeA = 1 => czerwony
// latticeB = 2 => niebieski
// latticeC = 3 => żółty
// latticeD = 4 => zielony
class Wykres extends JPanel implements MouseListener, MouseMotionListener {
    int wielkosc = 444;
    int square_size = 4;
    int liczba_czastek = wielkosc/square_size;
    int speed = 1;
    double bias_pion;
    double bias_poziom;
    double density = 0.25;
    int[][] lattice = new int[liczba_czastek][liczba_czastek];
    boolean rysowac = false;
    boolean klasteryzacja = true;
    boolean anihilacjaAA = false;
    boolean anihilacjaAB = false;
    int[][] lattice_gatunek = new int[liczba_czastek][liczba_czastek];
    public String narzedzie = "olowek";  // Dodanie public pozwala na dzialanie tej klasy w calym pliku main
    int kolor = 5;
    Point punkt1, punkt2;
    int[][] lattice_rysunek = new int[liczba_czastek][liczba_czastek];
    double density1 = 1, density2 = 1, density3 = 1, density4 = 1;
    int[] anihilacjaAB_stary = {1, 2, 3, 4};  // Stała
    int[] anihilacjaAB_nowy = {2, 3, 4, 1};  // Zmienia się, zależnie od wyboru użytkownika


    public Wykres() {
        super(true); // Włączenie double buffering w celu zmiejszenia migotania
        setBackground(Color.BLACK); 
        setBorder(BorderFactory.createLineBorder(Color.white, 4));
        initial();
        addMouseListener(this);
        addMouseMotionListener(this);
    }

    public void initial() {
        for (int i = 0; i < liczba_czastek; i++) {
            for (int j = 0; j < liczba_czastek; j++) {
                if (Math.random() < density) {
                    int liczba = (int) (4.0 * Math.random()) + 1;
                    double density0 = Math.random();
                    switch (liczba) {
                        case 1:
                            if (density1 >= density0) {
                                lattice[i][j] = 1;
                            }
                            else {
                                lattice[i][j] = 0;
                            }
                            break;
                        case 2:
                            if (density2 >= density0) {
                                lattice[i][j] = 2;
                            }
                            else {
                                lattice[i][j] = 0;
                            }
                            break;
                        case 3:
                            if (density3 >= density0) {
                                lattice[i][j] = 3;
                            }
                            else {
                                lattice[i][j] = 0;
                            }
                            break;
                        case 4:
                            if (density4 >= density0) {
                                lattice[i][j] = 4;
                            }
                            else {
                                lattice[i][j] = 0;
                            }
                            break;
                        default:
                            lattice[i][j] = 0;
                            break;
                    }
                } else {
                    lattice[i][j] = 0;
                }
                lattice_rysunek[i][j] = 0;
            }
        }
    }

    void step() {
        for (int counter = 0; counter < speed * liczba_czastek * liczba_czastek; counter++) {
            int x_old = (int) (liczba_czastek * Math.random());
            int y_old = (int) (liczba_czastek * Math.random());
            if (lattice[x_old][y_old] != 0 && lattice_rysunek[x_old][y_old] == 0) {
                int x = x_old;
                int y = y_old;
                if (lattice[x_old][y_old] == 1 || lattice[x_old][y_old] == 2){
                    double bias = bias_pion;
                    if (lattice[x_old][y_old] == 2) {
                        bias = -bias_pion;
                    }
                    double losowa = Math.random();
                    if (losowa < 0.25 + bias) {
                        x = x + 1;
                        if (x == liczba_czastek) {
                            x = 0;
                        }
                    } else if (losowa < 0.5) {
                        x = x - 1;
                        if (x == -1) {
                            x = liczba_czastek - 1;
                        }
                    } else if (losowa < 0.75) {
                        y = y + 1;
                        if (y == liczba_czastek) {
                            y = 0;
                        }
                    } else {
                        y = y - 1;
                        if (y == -1) {
                            y = liczba_czastek - 1;
                        }
                    }
                }
                if (lattice[x_old][y_old] == 3 || lattice[x_old][y_old] == 4){
                    double bias = bias_poziom;
                    if (lattice[x_old][y_old] == 4) {
                        bias = -bias_poziom;
                    }
                    double losowa = Math.random();
                    if (losowa < 0.25) {
                        x = x + 1;
                        if (x == liczba_czastek) {
                            x = 0;
                        }
                    } else if (losowa < 0.5) {
                        x = x - 1;
                        if (x == -1) {
                            x = liczba_czastek - 1;
                        }
                    } else if (losowa < 0.75 + bias) {
                        y = y + 1;
                        if (y == liczba_czastek) {
                            y = 0;
                        }
                    } else {
                        y = y - 1;
                        if (y == -1) {
                            y = liczba_czastek - 1;
                        }
                    }
                }

                int rodzaj_czastki = lattice[x_old][y_old];
                int rodzaj_czastki_nowy = lattice[x][y];
                if (klasteryzacja) {
                    int ilosc_sasiadow = 0;
                    if (x_old + 1 < liczba_czastek && lattice[x_old + 1][y_old] == rodzaj_czastki) {
                        ilosc_sasiadow++;
                    }
                    if (x_old - 1 >= 0 && lattice[x_old - 1][y_old] == rodzaj_czastki) {
                        ilosc_sasiadow++;
                    }
                    if (y_old + 1 < liczba_czastek && lattice[x_old][y_old + 1] == rodzaj_czastki) {
                        ilosc_sasiadow++;
                    }
                    if (y_old - 1 >= 0 && lattice[x_old][y_old - 1] == rodzaj_czastki) {
                        ilosc_sasiadow++;
                    }
                    if (ilosc_sasiadow > 2){
                        continue;
                    }
                }
                if (anihilacjaAA) {
                    if (lattice[x][y] == rodzaj_czastki && lattice_rysunek[x][y] == 0){
                        lattice[x_old][y_old] = 0;
                        lattice[x][y] = 0;
                    }
                }
                if (anihilacjaAB) {
                    // Jeśli będzie reakcja między A+B, B+C, C+D, D+A dochodzi do anihilacji
                    // if (((rodzaj_czastki == 1 && rodzaj_czastki_nowy == 2) || 
                    // (rodzaj_czastki == 2 && rodzaj_czastki_nowy == 3) || 
                    // (rodzaj_czastki == 3 && rodzaj_czastki_nowy == 4) || 
                    // (rodzaj_czastki == 4 && rodzaj_czastki_nowy == 1)) && lattice_rysunek[x][y] == 0) {
                    //     lattice[x_old][y_old] = 0;
                    //     lattice[x][y] = 0;
                    // }
                    if (rodzaj_czastki - 1 >= 0 && rodzaj_czastki - 1 < anihilacjaAB_stary.length && rodzaj_czastki_nowy - 1 >= 0 && rodzaj_czastki_nowy - 1 < anihilacjaAB_stary.length) {
                        if (anihilacjaAB_stary[rodzaj_czastki - 1] == anihilacjaAB_nowy[rodzaj_czastki_nowy - 1]) {
                            lattice[x_old][y_old] = 0;
                            lattice[x][y] = 0;
                        }
                    }

                }
                if (lattice[x][y] == 0) {
                    lattice[x][y] = lattice[x_old][y_old];
                    lattice[x_old][y_old] = 0;
                }
            }
        }
    }

    public void turn_off_on (int gatunek, boolean wlaczony) {
        if (wlaczony) {
            for (int i = 0; i < liczba_czastek; i++) {
                for (int j = 0; j < liczba_czastek; j++) {
                    if (lattice_gatunek[i][j] == gatunek) {
                        lattice[i][j] = gatunek;
                        lattice_gatunek[i][j] = 0;
                    }
                }
            }
        }
        else {
            for (int i = 0; i < liczba_czastek; i++) {
                for (int j = 0; j < liczba_czastek; j++) {
                    if (lattice[i][j] == gatunek) {
                        lattice_gatunek[i][j] = gatunek;
                        lattice[i][j] = 0;
                    }
                }
            }
        }
    }

    public void mouseReleased(MouseEvent e) {}
    public void mouseClicked(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public void mouseMoved(MouseEvent e) {}

    public void mousePressed(MouseEvent e) {
        punkt1 = new Point(e.getX() / square_size, e.getY() / square_size);
    }
    
    public void mouseDragged(MouseEvent e) {
        punkt2 = new Point(e.getX() / square_size, e.getY() / square_size);
    
        drawLine(punkt1.x, punkt1.y, punkt2.x, punkt2.y, kolor);
        punkt1 = punkt2;
        repaint();
    }
    
    // Algorytm Bresenhama
    private void drawLine(int x1, int y1, int x2, int y2, int kolor) {
        int dx = Math.abs(x2 - x1);
        int dy = Math.abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx - dy;
    
        while (true) {
            if (x1 >= 0 && x1 < liczba_czastek && y1 >= 0 && y1 < liczba_czastek) {
                if (narzedzie == "olowek") {
                    lattice[x1][y1] = kolor;
                    lattice_rysunek[x1][y1] = kolor;
                } else {
                    lattice[x1][y1] = 0;
                    lattice_rysunek[x1][y1] = 0;
                }
            }
    
            if (x1 == x2 && y1 == y2) {
                break;
            }
    
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }

    public void paintComponent(Graphics graf){
        super.paintComponent(graf);
        for (int i = 0; i < liczba_czastek; i++) {
            for (int j = 0; j < liczba_czastek; j++) {
                if (lattice[i][j] == 1) {
                    graf.setColor(Color.red);
                    graf.fillRect(i * square_size, j * square_size, square_size, square_size);
                } else if (lattice[i][j] == 2) {
                    graf.setColor(Color.blue);
                    graf.fillRect(i * square_size, j * square_size, square_size, square_size);
                } else if (lattice[i][j] == 3) {
                    graf.setColor(Color.yellow);
                    graf.fillRect(i * square_size, j * square_size, square_size, square_size);
                } else if (lattice[i][j] == 4) {
                    graf.setColor(Color.green);
                    graf.fillRect(i * square_size, j * square_size, square_size, square_size);
                } else if (lattice[i][j] == 5) {
                    graf.setColor(Color.white);
                    graf.fillRect(i * square_size, j * square_size, square_size, square_size);
                }

            }
        }
        if (rysowac) {
            step();
            try {
                Thread.sleep(20); // sleep for 10 msec
            } catch (InterruptedException t){}
            repaint();
        }
    }
}