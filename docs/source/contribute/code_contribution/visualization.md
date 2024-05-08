# Visualization

Multiple things can be visualized in PeTrack. Detected points, trajectories (projected to the ground), detected markers, etc...

The video display is build around the QGraphicsView Framework. Check out the [official Docs](https://doc.qt.io/qt-5/graphicsview.html).

A QGraphicsView is a view on multiple QGraphicItems. These Items all implement functions to determine the space [^1] they take up (boundingRect()) and to paint themselves (paint()). The paint method gets invoked by the GraphicsView these items are part of.

Every single visualization is contained in its own QGraphicsItem-Derivative, implementing the boundingRect() and paint() method. You can use existing items as blueprint for a new one. You need to create an instance of your new item and add it to the scene of PeTracks GraphicView in the Constructor of Petrack. Just search for the part with declarations like the following and firmly integrate your own item:

```cpp
mBackgroundItem = new BackgroundItem(this);
mBackgroundItem->setZValue(2.2); // Item with bigger values covers over items with smaller values
mBackgroundItem->setVisible(false);
```

```cpp
mScene->addItem(mBackgroundItem);
```


************

[^1]: The bounding rect also determines the position