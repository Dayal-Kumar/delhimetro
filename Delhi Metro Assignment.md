# Delhi Metro Assignment

Dayal Kumar (2020MT10797)

### Problem Statement

To build a system to track shortest distance between any two stations of the Delhi Metro. The system will be a client side web app written in C++ compiled to WebAssembly using emscripten with wgpu target. 

Methods to be supported -

1. Login for the admin 
2. Add station by the admin
3. Add links between stations by the admin
4. Get the distance and path between two stations by anyone

The dependencies are:

1. [Dear ImGui]([https://github.com/ocornut/imgui](https://github.com/ocornut/imgui))
2. [Header-only constexpr SHA256 C++ 14 literal]([https://github.com/aguinet/sha256_literal](https://github.com/aguinet/sha256_literal))

## How to Run

Go to the root of the directory and run “make” in the terminal. It builds the program and spins up a server on [http://localhost:8000](http://localhost:8000) . Go to this url to test the application.

The application also has some data preloaded to test the functionality.

## Challenge 1 : Getting the rendering to work

As I have never done UI in C++, although I have tried earlier and gave up as it was quite overwhelming, I tried to find simple libraries. I came across Dear Imgui, which is an immediate mode ui rendering library (immediate means that the renderer just loops the function I provide it, does not maintain any state). 

This [video]([https://www.youtube.com/watch?v=vWXrFetSH8w](https://www.youtube.com/watch?v=vWXrFetSH8w)) helped me to get started with the initial template. I used the emscripten_wgpu example. 

## Challenge 2 : Creating the Search Bar

There were many difficulties. Most of them arose due to my decision to do early optimisation. I decided to implement a trie in a non-conventional way, storing the nodes and strings in arrays for efficient cache performance.

I created the search bar component having two sub-components, one the TextBox and the other a ListBox to show the completions. All the stations are stored in a trie which is queried to get a list of completions. This list of completions is updated every time the TextBox content is edited.

There were several issues and decisions I have taken, which are as follows:

- Should the trie be queried with the **whole new string** whenever the content is edited, or to use the difference between previous string and current string to **incrementally update** the root of the required subtree? I decided to do the former as there is no guarantee about the two strings to any similar, they can be wildly different. Moreover, the first one is easier to implement.
- **Should the nodes of the trie be malloced individually?** I decided to hold all the nodes in a vector and the trie pointers are just the indices in the vector.
- One of the bug was that I was **storing pointers to elements of the vector** (I have done similar thing in C projects, but there the array was not being resized), which is a bug as the vector may at some point get resized and thus be copied to some other location and hence the stored pointers become invalid.
- As Dear Imgui uses Immediate Mode Rendering, I was assuming that there would be **no callbacks and events,** but I was wrong. But I still feel events and callbacks are just an abstraction and work exactly same as the non-callback way.
- The interface provided by Dear Imgui for the **ComboBox** is pretty limited. I was not able to get the desired functionality of the search bar. I just desired to merge the Text Input and ComboBox to get the desired results, but that did not work.
    - I also tried replacing the combobox with a popup but the **Popup** takes over the keyboard focus from the text input field, which is undesirable.
    - So, I settled on to using **ListBox** as the container to hold the completions.
- There were several edge cases which needed to be considered while building the search bar.
- The ComboBox is being used in multiple components. I wanted for each component to show the ListBox if the Text Field is focused. This was difficult as if multiple components took the focus one by one, more than one ListBox would be activated which is undesirable. So, I had to use the idea of **turns** so that different ComboBox’s could share the turn of showing the ListBox. This was complicated because the focus flag is also used for selecting from one of the completions and populating the text field.

## Challenge 3 : Computing Shortest Paths

I used Dijkstra’s Algorithm for computing the shortest paths between two stations. One bug I faced in which when a link was added the edge costs were not being updated. It turned out to be a typo, I missed to mark the graph as a static variable, so in each frame it was getting the graph initialised with original values.

## Challenge 4 : Creating Admin Login

This was fairly easy as I did it at last (being used to ImGui). One thing was to store the password. The password should not be stored as a string in the program, as one can find all the strings in an executable using some utility. So, I decided to generate the password hash at compile time and store only the hash. Also, the salt is stored as a string which is harmless. 

We would not have needed all of this if we ensured that the password is fairly complicated, but it is impractical usually. I used [this]([https://github.com/aguinet/sha256_literal](https://github.com/aguinet/sha256_literal)) compile time sha256 library to compute the hash. 

## Error Handling

- **Invalid Stations**: On entering station names, the trie is checked if it contains that station, if not, an error is shown as “Invalid stations”.
- **Wrong Password:** On entering wrong password, similar error is shown.