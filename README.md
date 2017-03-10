# ObjectOrientedDesign-PreferringCompositionOverInheritance
This is a short paper describing a composition based "Has-A" design for the classic "Worms" simulation.

# Abstract
A goal of object-oriented programming is to maximize programmer productivity by reducing lifetime software development and maintenance costs. The principal technique used to achieve the goal is object reuse. An object that is reused saves the programmer time because the object would otherwise need to be reimplemented for each new project. Another potential benefit of reusing objects is that when new features are required or bugs are identified, changes can often be made to a small number of objects, and those changes benefit other projects that rely on the same objects. Most importantly, by reusing objects, fewer total lines of code are written to solve each new problem, and that means there are fewer lines of code to maintain as well.

Several guiding principles of design promote object reuse, and foremost among those principles is minimization of coupling. Coupling refers to dependencies between objects. Whenever such dependencies exist, they reduce opportunities for reusing the objects independently. Coupling frequently manifests as situations in which changes to one object necessitate changes to other objects or when one object cannot be reused without also requiring reuse of other objects. Coupling also applies to subsystems within large systems of objects. To achieve object reuse, it’s important to look for designs that avoid coupling whenever possible.

This paper examines one of the most common and most severe sources of coupling, object oriented inheritance, and describes use of designs emphasizing composition relationships in preference to inheritance relationships between objects. The classic but trivial “Worms” simulation program is used as a case study. A brief overview of the “Worms” simulation is provided as a basis and case study for application of composition in preference to object oriented inheritance.
