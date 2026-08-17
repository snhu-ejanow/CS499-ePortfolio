// Developer: Eryk Janowski
import com.mongodb.client.MongoClient;
import com.mongodb.client.MongoClients;
import com.mongodb.client.MongoCollection;
import com.mongodb.client.MongoDatabase;
import org.bson.Document;

import java.awt.BorderLayout;
import java.awt.CardLayout;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.FlowLayout;
import java.awt.GridLayout;
import java.awt.HeadlessException;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;
import java.awt.Color;

import java.util.ArrayList;

public class SlideShow extends JFrame {
	// Declare Variables
	// Swing content containers
	private JPanel slidePane;
	private JPanel textPane;
	private JPanel buttonPane;
	private JPanel navbarPane;
	private JPanel slideWrapper;
	private JPanel navWrapper;
	private CardLayout card;
	private CardLayout cardText;
	private JButton btnFirst;
	private JButton btnNext;
	private JButton btnPrev;
	private JButton btnLast;
	private JLabel lblSlide;
	private JLabel lblTextArea;
	
	// User window vars
	private int imageWidth;
	private int imageHeight;
	private int index;
	
	// Arrays to store database import
	private ArrayList<String> imagePaths = new ArrayList<>();
	private ArrayList<String> imageTitles = new ArrayList<>();
	private ArrayList<String> imageSubs = new ArrayList<>();
	private ArrayList<String> imageSubsLong = new ArrayList<>();
	private ArrayList<Boolean> imageShowingLong = new ArrayList<>();

	// Create the application
	public SlideShow() throws HeadlessException {
		importMongoData();
		initComponent();
	}
	
	private void importMongoData() {
		// Path of mongoDB database being used
		String mongoPath = "mongodb://localhost:27017";
		
		// Safely attempt to import database data into arrays
		try (MongoClient mongoClient = MongoClients.create(mongoPath)) {
			MongoDatabase database = mongoClient.getDatabase("SlideShowDB");
			MongoCollection<Document> collection = database.getCollection("slides");
			
			for (Document doc : collection.find()) {
				imagePaths.add(doc.getString("path"));
				imageTitles.add(doc.getString("title"));
				imageSubs.add(doc.getString("sub"));
				imageSubsLong.add(doc.getString("subLong"));
				imageShowingLong.add(false);
			}
		} catch (Exception e) {
			System.err.println("Failed to connect to MongoDB");
			e.printStackTrace();
			System.exit(1);
		}
	}

	// Initialize the contents of the frame.
	private void initComponent() {
		// Get screen size and set initial window to 70%
		Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
		setSize((int)(screenSize.width * 0.7), (int)(screenSize.height * 0.7));
		
		// Set image width to 40% of screen width and a 16:10 aspect ratio
		imageWidth = (int)(screenSize.width * 0.4);
		imageHeight = (int)(imageWidth * 0.625);
		
		// Set other JFrame attributes
		setLocationRelativeTo(null);
		setTitle("Top " + imagePaths.size() + " Destinations SlideShow");
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		getContentPane().setLayout(new BorderLayout(10, 10));
		
		// Initialize variables
		// Card
		card = new CardLayout();
		cardText = new CardLayout();
		
		// Slide
		slidePane = new JPanel(card);
		
		// Text
		textPane = new JPanel(cardText);
		textPane.setBackground(Color.LIGHT_GRAY); //Light Gray to improve readability
		textPane.setPreferredSize(new Dimension(imageWidth, imageHeight));
		
		// Navbar setup
		navbarPane = new JPanel(new GridLayout(imagePaths.size(), 1, 0, 15));
		for (int i = 0; i < imagePaths.size(); i++) { 
			int slideIndex = i;
			JButton btnJump = new JButton(String.valueOf(i+1) + ". " + imageTitles.get(i));
			btnJump.addActionListener(new ActionListener() {
				@Override
				public void actionPerformed(ActionEvent e) {
					jumpToSlide(slideIndex);
				}
			});
			navbarPane.add(btnJump);
		}
		
		// Buttons
		buttonPane = new JPanel(new FlowLayout(FlowLayout.CENTER, 20, 10));
		
		btnFirst = new JButton("First");
		btnNext = new JButton("Next");
		btnPrev = new JButton("Previous");
		btnLast = new JButton("Last");
		
		// Button listeners
		btnFirst.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				changeSlide("First");
			}
		});
		btnNext.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				changeSlide("Next");
			}
		});
		btnPrev.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				changeSlide("Prev");
			}
		});
		btnLast.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				changeSlide("Last");
			}
		});
		
		// Buttons ordered to align with slideshow flow
		buttonPane.add(btnFirst);
		buttonPane.add(btnPrev);
		buttonPane.add(btnNext);
		buttonPane.add(btnLast);
		
		navWrapper = new JPanel(new BorderLayout());
		navWrapper.add(navbarPane, BorderLayout.CENTER);
		navWrapper.add(buttonPane, BorderLayout.SOUTH);
		
		slideWrapper = new JPanel();
		slideWrapper.add(slidePane);
		slideWrapper.add(textPane);

		getContentPane().add(navWrapper, BorderLayout.WEST);
		getContentPane().add(slideWrapper, BorderLayout.CENTER);
		
		// Set index
		index = 0;
		
		// Load slide content
		loadSlides();
	}

	// Navbar jump functionality
	private void jumpToSlide(int slideIndex) {
		card.show(slidePane, "card" + slideIndex);
		cardText.show(textPane, "cardText" + slideIndex);
		index  = slideIndex;
	}
	
	// Button jump functionality
	private void changeSlide(String move) {
		switch (move) {
			case "First":
				card.first(slidePane);
				cardText.first(textPane);
				index = 0;
				break;
			case "Next":
				card.next(slidePane);
				cardText.next(textPane);
				if (index + 1 == imagePaths.size()) {
					index = 0;
				} else {
					index++;
				}
				break;
			case "Prev":
				card.previous(slidePane);
				cardText.previous(textPane);
				if (index - 1 == -1) {
					index = imagePaths.size()-1;
				} else {
					index--;
				}
				break;
			case "Last":
				card.last(slidePane);
				cardText.last(textPane);
				index = imagePaths.size()-1;
				break;
		}
	}
	
	// Method to load slides
	private void loadSlides() {
		for (int i = 0; i <= imagePaths.size() - 1; i++) {
			// Initialize variables
			lblSlide = new JLabel();
			lblTextArea = new JLabel();
			
			// Load images using HTML
			lblSlide.setText("<html><img src='" + getClass().getResource(imagePaths.get(i)) + "'width='" + imageWidth + "'height='" + imageHeight + "'></html>");
			
			// Set short text and mouse listener for clicks to switch to long text
			lblTextArea.setPreferredSize(new Dimension(imageWidth, imageHeight));
			lblTextArea.setVerticalAlignment(JLabel.TOP);
			lblTextArea.setText("<html><body>" + imageSubs.get(i) + "<font size='4'><br><br>(Click for more information)</font></body></html>");
			lblTextArea.addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    JLabel labelClicked = (JLabel) e.getSource();
                    if (imageShowingLong.get(index)) {
                        labelClicked.setText("<html><body>" + imageSubs.get(index) + "<font size='4'><br><br>(Click for more information)</font></body></html>");
                        imageShowingLong.set(index, false);
                    } else {
                        labelClicked.setText("<html><body>" + imageSubs.get(index) + imageSubsLong.get(index) + "</body></html>");
                        imageShowingLong.set(index,  true);
                    }
                    labelClicked.revalidate();
                    labelClicked.repaint();
                }
            });
			
			// Scrollpane in case long description extends beyond the size of the text pane and text label
			JScrollPane scrollPane = new JScrollPane(lblTextArea);
			scrollPane.setVerticalScrollBarPolicy(ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
			scrollPane.setHorizontalScrollBarPolicy(ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
			scrollPane.setBorder(null);
			scrollPane.getViewport().setBackground(Color.LIGHT_GRAY);
			
			slidePane.add(lblSlide, "card" + i);
			textPane.add(scrollPane, "cardText" + i);
		}
	}
	
	// Launch the application.
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			@Override
			public void run() {
				SlideShow ss = new SlideShow();
				ss.setVisible(true);
			}
		});
	}
}