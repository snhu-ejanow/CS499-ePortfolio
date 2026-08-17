# CS499-ePortfolio

## Overview and Self-Assessment
Completing the coursework for CS 499 has given me a new appreciation for the process of reviewing and polishing my work. The process has included identifying and understanding mistakes and decisions made during the original development process, and has allowed me a deeper understanding of my strengths as a developer. Throughout the entire CS program, I have also been introduced to several other skills and abilities. I have developed the ability to design projects to follow AGILE and SCRUM methodology and write code in a way that allows team members and collaborators to understand the purpose of my software and effectively critique it. Through AGILE training, I have learned the best practices for gathering stakeholder priorities, turning them into actionable data points, and then communication development progress and expectations back to those project stakeholders. For efficient programming, I have learned how to implement algorithms and data structure processes to effectively handle large amounts of data in a way that is performance-light and bug-free. I have learned the methodology for designing comfortable, easy-to-use User-facing interfaces that provide desired functionality in a clean and readable manner. And lastly, I have developed a security mindset that focuses on keeping sensitive data secure and away from program functions that could expose it to unauthorized individuals, and have learned how to implement database management features, encryption services, and vulnerability assessments to achieve those goals.

The two artifacts presented below were chosen because they offered ample opportunity to display the strengths and skills mentioned above. They have allowed me to display my advancements in user-facing design, efficient programming, use of coding best-practices, ability to implement algorithms and data structures, usage of databases, and security mindset by iterating on the original implementations of these artifacts and enhancing them with features that build upon their base functionality and elevate them beyond their original design goals.

## Code Review
The Code Review for the original artifacts can be found [here](https://snhu-my.sharepoint.com/:v:/g/personal/eryk_janowski_snhu_edu/IQDOVX5oBIGzTrq46sXnsFriAWPwK4PasaZnumwhErnyP6c?nav=eyJyZWZlcnJhbEluZm8iOnsicmVmZXJyYWxBcHAiOiJPbmVEcml2ZUZvckJ1c2luZXNzIiwicmVmZXJyYWxBcHBQbGF0Zm9ybSI6IldlYiIsInJlZmVycmFsTW9kZSI6InZpZXciLCJyZWZlcnJhbFZpZXciOiJNeUZpbGVzTGlua0NvcHkifX0&e=6CxOLb).

## Enhancement 1 - Software Design and Engineering
The artifact used is a Health Resort slideshow made using Java Swing for CS 250, a class focusing on the AGILE methodology. The goal of this artifact was to display five health resort locations with short descriptions in a slide show format.

The original artifact can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20Slide%20Show%20Original).

This artifact was chosen because it provided the opportunity to display improvements in end-user design philosophy through enhancements in both user-facing UI and backend programming efficiency. The original artifact utilized hardcoded window and image sizes, lacked dynamic scaling functionality, and generally was not user-friendly and difficult to traverse.

It has been improved in the following ways:
- The window and image dynamically scale according to the user's screen size
- All elements are now dynamic and will no longer obscure each other when the window is resized
- A sidebar with quick-navigation buttons allows users to jump to any point in the slideshow
- "First" and "Last" buttons have been added to allow the user to skip to the beginning or end
- All buttons have been moved to below the quick navigation bar
- The text box will either be below or to the right of the image, whichever area has more space
- The text box can be interacted with to reveal a longer description of each health resort
- Code defining component functionality (such as buttons) has been moved outside of component initialization

The enhanced artifact can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20Slide%20Show%20Enhanced).

## Enhancement 2 - Algorithms & Data Structures
The artifact used is an OpenGL C++ render of the Street-facing side of the US Federal Reserve Back in Downtown Chicago, created for CS 330. The goal is to display proficiency in OpenGL be creating a scene containing several different types of meshes, textures, materials, and lighting and objects comprised of multiple different types of meshes.

The original C++ artifact files can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20OpenGL%20Render%20Original). The main files of focus are SceneManager.cpp and SceneManager.h

This artifact was chosen because it provided the opportunity to display improvements in data management, algorithm usage, and the ability to write efficient and future-proofed code. The original artifact was compromised of a large amount of redundant and repeated lines of code, did not utilize algorithms to reduce repetition, and was space inefficient.

It has been improved in the following ways:
- The data for Lights, Materials, Textures, and Meshes has been moved into data structures of Nested vectors and object types
- All initializations of Lights, Materials, Textures, and Meshes have been moved to algorithm functions to eliminate redundant initialization code
- Data structures and algorithms designed and commented in such a way to allow anyone to add new features to the render by simply inserting a new data entry
- The file size of SceneManager.cpp reduced by 57% from 77 KB to 33 KB due to the above

The enhanced artifact's C++ files can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20OpenGL%20Render%20Enhanced). Again, the files to focus on are SceneManager.cpp and SceneManager.h

## Enhancement 3 - Databases
The artifact used is the same Java Swing-based Health Resort slideshow utilized for Enhancement 1.

The original artifact can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20Slide%20Show%20Original).

This artifact was chosen because it provided the opportunity to display improvements in data storage and management, streamlining of development processes, and accounting for data security. The original artifact hardcoded the data used for the content directly in the code, which unnecessarily bloats the size of the program file and exposes potentially sensitive data to anyone with access to the Java file.

It has been improved in the following ways:
- All slide content data has been moved to an external MongoDB database
- A new function has been implemented to import the MongoDB data to various dynamic vectors, which are then used to initialize the content for each card
- The program dynamically handles slide initialization, accounting for a variable number of entries imported from MongoDB and adjusts content displayed automatically
- As a result of the above, adding new entries to the slideshow only requires a new entry in the database and no longer requires additional hardcoding
- By moving hardcoded data to an external database, it is no longer exposed to anyone with the Java executable

The enhanced artifact can be found [here](https://github.com/snhu-ejanow/CS499-ePortfolio/tree/main/Janowski%20-%20Slide%20Show%20Enhanced).

## Github Repository
[CS-499 ePortfolio](https://github.com/snhu-ejanow/CS499-ePortfolio)
