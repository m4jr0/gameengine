# Resources & Inspirations

A collection of talks, books, and articles that inspired **Comet**, or helped me explore topics like game engine programming, rendering, and low-level systems.

## Essential Reads

These are the key resources I recommend first: a great starting point for anyone interested in engine programming.

### C++ Programming
  * **Scott Meyers** - [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)
    * A classic on modern C++ design. Focused on C++14, but its lessons remain timeless.
### Game Engine
  * **Jason Gregory** - [Game Engine Architecture](https://www.gameenginebook.com/)
    * The ABSOLUTE bible for understanding how game engines work. Comprehensive and deeply practical.
### Concurrency & Multithreading
  * **Anthony Williams** - [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
    * The best book about multithreading and synchronization I've read.
### Rendering
  * OpenGL
    * **Joey de Vries** - [LearnOpenGL](https://learnopengl.com)
      * The best free OpenGL guide out there: structured, practical, and example-driven.
  * Vulkan
    * **Alexander Overvoorde** - [Vulkan Tutorial](https://vulkan-tutorial.com/)
    * **vblanco20-1** - [Vulkan Guide](https://vkguide.dev/)
      * Excellent introductions to modern Vulkan rendering pipelines.
  * Generalities
    * **Jean-Colas Prunier** - [Scratchapixel 4.0](https://www.scratchapixel.com/)
      * One of the best sites to understand computer graphics from first principles.
### Design Patterns
  * **Robert Nystrom** - [Game Programming Patterns](https://gameprogrammingpatterns.com)
    * A must-read for anyone structuring gameplay or engine code.
### Math
  * **Eric Lengyel** - [Foundations of Game Engine Development](https://foundationsofgameenginedev.com/) (Vol. 1: Mathematics)
    * Crystal-clear mathematical explanations with engine-focused examples.
### Physics
  * **Christer Ericson** - [Real-Time Collision Detection](https://www.crcpress.com/Real-Time-Collision-Detection/Ericson/p/book/9781558607323)
    * A timeless reference for collision and geometry logic.

## Extended Reading

*(Not exhaustive -- I've lost track of many more!)*
Below are additional articles, papers, and resources grouped by topic.

### Game Engine Architecture

* **Jason Gregory** - [Game Engine Architecture](https://www.gameenginebook.com/)
* **Travis Vroman** — [Kohi Engine](https://kohiengine.com/)
* **Isetta Engine Team** - [Isetta Engine](https://isetta.io/)
* **Squirrel Eiserloh** - [Overwatch Gameplay Architecture and Netcode](https://www.gdcvault.com/play/1024365/Overwatch-Gameplay-Architecture-and-Netcode)

### Job Systems & Concurrency

* **Christian Gyrling** - [Parallelizing the Naughty Dog Engine Using Fibers](https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine)
* **turanszkij** - [Simple job system using standard C++](https://wickedengine.net/2018/11/24/simple-job-system-using-standard-c/)
* **Rismosch** - [Building a JobSystem (Rust)](https://www.rismosch.com/article?id=building-a-job-system)
* **Molecular Musings** - [Job System 2.0: Lock-Free Work Stealing – Part 3](https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/)
* **Jeremy Laumon** - [Job System](http://danglingpointers.com/post/job-system/)
* **Jeremy Laumon** - [Job System #2](http://danglingpointers.com/post/job-system-2/)
* **Tobias Persson** - [Fiber-based job system](https://ruby0x1.github.io/machinery_blog_archive/post/fiber-based-job-system/index.html)
* **krzysztofmarecki** - [Fiber based Job System](https://github.com/krzysztofmarecki/JobSystem)
* **ck** - [The Concurrency Kit](https://github.com/concurrencykit/ck)
* **Jeff Preshing** - [An Introduction to Lock-Free Programming](https://preshing.com/20120612/an-introduction-to-lock-free-programming/)
* **Christian Gyrling** - [Cache Coherency and Multi-Core Programming](https://www.gdcvault.com/play/1022248/Parallelizing-the-Naughty-Dog-Engine-Using)
* **Keir Fraser** - [Practical Lock-Freedom](https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf)
* **Paul E. McKenney et al.** - [Is Parallel Programming Hard, and If So, What Can You Do About It?](https://kernel.org/pub/linux/kernel/people/paulmck/perfbook/perfbook.html)
* **Nodari Kankava** - [Exploring the Efficiency of Multi-Word Compare-and-Swap](https://www.diva-portal.org/smash/get/diva2:1635674/FULLTEXT01.pdf)
* **Peter Pirkelbauer et al.** - [A Portable Lock-Free Bounded Queue](https://doi.org/10.4230/LIPIcs.DISC.2019.28)
* **Rachid Guerraoui et al.** - [Efficient Multi-word Compare and Swap](https://doi.org/10.48550/arXiv.2008.02527)
* **Ruslan Nikolaev** - [A Scalable, Portable, and Memory-Efficient Lock-Free FIFO Queue](https://doi.org/10.48550/arXiv.1908.04511)
* **Steven Feldman et al.** - [A Practical Wait-Free Multi-Word Compare-and-Swap Operation](https://www.researchgate.net/profile/Pierre-Laborde/publication/276848211_A_Practical_Wait-Free_Multi-Word_Compare-and-Swap_Operation/links/5e45b1e5a6fdccd965a2f51f/A-Practical-Wait-Free-Multi-Word-Compare-and-Swap-Operation.pdf)

### Fibers, Coroutines & Low-level

* **Bradley Chatha** - [Implementing Fibers](https://www.linkedin.com/pulse/implementing-fibers-bradley-chatha)
* **Jiayin Cao** - [Fibers in C++: Understanding the Basics](https://agraphicsguynotes.com/posts/fiber_in_cpp_understanding_the_basics/)
* **Dale Weiler** - [Fibers, Oh My!](https://graphitemaster.github.io/fibers/)
* **Lewis Baker** - [C++ Coroutines: Understanding the Compiler Transform](https://lewissbaker.github.io/2022/08/27/understanding-the-compiler-transform)
* **Lewis Baker** - [C++ Coroutines: Understanding operator co_await](https://lewissbaker.github.io/2017/11/17/understanding-operator-co-await)
* **Oliver Kowalke** - [Boost.Context: Fibers Documentation](https://www.boost.org/doc/libs/1_84_0/libs/context/doc/html/context/ff.html)
* **Microsoft Corp.** [x64 Calling Convention (MSVC)](https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention)
* **Jean-Christophe Filliâtre** - [Notes on x86-64 Programming](https://www.lri.fr/~filliatr/ens/compil/x86-64.pdf)
* **William Woodruff** - [How x86_64 Addresses Memory](https://blog.yossarian.net/2020/06/13/How-x86_64-addresses-memory)
* **Robert Harvey** - [Fibers vs. Coroutines vs. Green Threads](https://softwareengineering.stackexchange.com/questions/254140/is-there-a-difference-between-fibers-coroutines-and-green-threads-and-if-that-i)
* **Alex Darby** - [C/C++ Low-Level Curriculum](https://github.com/alexdarby/LowLevelCurriculum)

### Concurrency & Synchronization Problems

* **Anthony Williams** - [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action)
* **Dafuq is that** - [Dining Philosophers: Hygiene Solution](https://dafuqis-that.com/2020/04/10/dining-philosophers-an-intuitive-interpretation-of-the-hygiene-solution/)
* **Michael L. Scott** - [Distributed Programming Assignment – University of Rochester](https://www.cs.rochester.edu/u/scott/courses/456/assignments/philos.html)
* **Aditya Y Bhargava** - [The Dining Philosophers Problem With Ron Swanson](https://www.adit.io/posts/2013-05-11-The-Dining-Philosophers-Problem-With-Ron-Swanson.html)

### Rendering & Graphics

* **Joey de Vries** - [LearnOpenGL](https://learnopengl.com)
* **Alexander Overvoorde** - [Vulkan Tutorial](https://vulkan-tutorial.com/)
* **vblanco20-1** - [Vulkan Guide](https://vkguide.dev/)
* **Pawel Lapinski** - [Vulkan Cookbook](https://www.packtpub.com/product/vulkan-cookbook/9781786468154)
* **Parminder Singh** - [Learning Vulkan](https://www.packtpub.com/product/learning-vulkan/9781786469809)
* **Matthias Bauchinger** - [Designing a Modern Rendering Engine](https://matt77hias.medium.com/)
* **Iago Toral** - [Working with Lights and Shadows - Part II: The Shadow Map](https://blogs.igalia.com/itoral/2017/07/30/working-with-lights-and-shadows-part-ii-the-shadow-map/)
* **Rye Terrell** - [Instanced Line Rendering, Part I](https://wwwtyro.net/2019/11/18/instanced-lines.html)
* **Ankit Singh Kushwah** - [Skeletal Animation](https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation)
* **Jean-Colas Prunier** - [Scratchapixel 4.0](https://www.scratchapixel.com/)
* **Grant Sanderson & Ben Eater** [Visualizing Quaternions (explorable video)](https://eater.net/quaternions)

### Memory & Allocation

* **Christian Gyrling** - [Are We Out of Memory?](https://www.swedishcoding.com/2008/08/31/are-we-out-of-memory/)
* **Chris Wellons** - [Purgeable Memory Allocations for Linux](https://nullprogram.com/blog/2019/12/29/)
* **Jeff Kiah** - [Game Engine Containers - handle_map](https://www.gamedev.net/tutorials/_/technical/general-programming/game-engine-containers-handle-map-r4495/)
* **Kurt Guntheroth** - [Chapter 4: Optimize String Use (O'Reilly)](https://www.oreilly.com/library/view/optimized-c/9781491922057/ch04.html)
* **itecnotes** - [std::string with a Custom Allocator](https://itecnotes.com/tecnote/c-stdstring-with-a-custom-allocator/)
* **Julien Jorge** - [Effortless Performance Improvements in C++: string_view](https://julien.jorge.st/posts/en/effortless-performance-improvements-in-cpp-std-string_view/)

### Algorithms

* **Robert Nystrom** - [Game Programming Patterns](https://gameprogrammingpatterns.com)
* **Robert Sedgewick & Kevin Wayne** - [Algorithms](https://algs4.cs.princeton.edu/home/)
* **Ulrich Drepper** - [What Every Programmer Should Know About Memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf)
* **Milan Stevanović** - [C and C++ Compiling](https://www.oreilly.com/library/view/advanced-c-and/9781430266679/)
* **Scott Meyers** - [Effective Modern C++](https://www.oreilly.com/library/view/effective-modern-c/9781491908419/)
* **Scott Meyers** - [Effective STL](https://www.oreilly.com/library/view/effective-stl/0321334876/)
* **Kip Irvine** - [Assembly Language for x86 Processors](https://www.pearson.com/en-us/subject-catalog/p/assembly-language-for-x86-processors/P200000006122)

### Math

* **Eric Lengyel** - [Foundations of Game Engine Development](https://foundationsofgameenginedev.com/) (Vol. 1: Mathematics, Vol. 2: Rendering)

### Physics

* **Christer Ericson** - [Real-Time Collision Detection](https://www.crcpress.com/Real-Time-Collision-Detection/Ericson/p/book/9781558607323)

### Complexity

* **Emily Marshall** - [Computational Complexity of Fibonacci Sequence](https://www.baeldung.com/cs/fibonacci-computational-complexity)
* **Adarsh Tyagi** - [Fibonacci in Constant Time](https://medium.com/@adarshtyagi/fibonacci-in-constant-time-945546c9e64c)

### ECS & Data-Oriented Design

* **Sander Mertens** - *Building an ECS* series
  * [Where are my Entities and Components](https://ajmmertens.medium.com/building-an-ecs-1-where-are-my-entities-and-components-63d07c7da742)
  * [Archetypes and Vectorization](https://ajmmertens.medium.com/building-an-ecs-2-archetypes-and-vectorization-fe21690805f9)
  * [Storage in Pictures](https://ajmmertens.medium.com/building-an-ecs-storage-in-pictures-642b8bfd6e04)

### Compression

* **Jonathan Blow** - [Scalar Quantization](http://number-none.com/product/Scalar%20Quantization/index.html)

### Logging, IO & Utilities

* **Felix Palmen** - [A Beginner's Guide Away From scanf()](https://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html)

### Misc

* **John Carmack** - [Functional Programming in C++](http://sevangelatos.com/john-carmack-on/)
