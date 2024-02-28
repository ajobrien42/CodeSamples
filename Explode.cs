/* Explode.cs
 * 
 * Applied to a multi-component object in Unity, toggles between the standard and 3D exploded view of the object with a smooth animation.
 * Demoed on a sandwich and a jet engine.
*/

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Explode : MonoBehaviour
{
    public bool exploded { get; private set; }
    public bool moving { get; private set; }

    private float speed = 10f;
    private float max_distance = 0;
    private List<float> distances;

    private List<Vector3> original_positions;
    private List<Vector3> exploded_positions;
    private Vector3 explosion_center;


    void Start()
    {
        original_positions = new List<Vector3>(transform.childCount);
        exploded_positions = new List<Vector3>(transform.childCount);
        distances = new List<float>(transform.childCount); // how far each part moves
        exploded = false;

        // set initial positions of all parts
        //foreach (Transform part in transform)
        for (int i = 0; i < transform.childCount; i++)
        {
            original_positions.Add(transform.GetChild(i).position);
        }

        // select the position of the first part as the center of the explosion -- can be any position!
        explosion_center = original_positions[0];
        //explosion_center = new Vector3(2.36f, 0f, -3f);

        for (int i = 0; i < original_positions.Count; i++)
        {
            // set exploded positions of all parts
            exploded_positions.Add(explosion_center + (original_positions[i] - explosion_center) * 3);

            // calculate distances and max_distance for use in adjusting part speeds
            distances.Add(Vector3.Distance(exploded_positions[i], original_positions[i]));
            if (distances[i] > max_distance)
            {
                max_distance = distances[i];
            }
        }
    }
    

    void Update()
    {
        int i;
        int parts_done;

        if (Input.GetMouseButtonDown(0))
        {
            moving = true;
        }

        if (moving)
        {
            i = 0;
            parts_done = 0;

            if (!exploded) // explode
            {
                foreach (Transform part in transform)
                {                    
                    part.position = Vector3.MoveTowards(part.position,
                                                        exploded_positions[i],
                                                        (distances[i] / max_distance) * speed * Time.deltaTime);

                    if (part.position == exploded_positions[i])
                    {
                        parts_done++;
                    }

                    i++;
                }

                // check if all parts have reached their destinations
                if (parts_done == transform.childCount)
                {
                    moving = false;
                    exploded = true;
                }
            }
            else // unexplode
            {
                foreach (Transform part in transform)
                {
                    part.position = Vector3.MoveTowards(part.position,
                                                        original_positions[i],
                                                        (distances[i]/max_distance) * speed * Time.deltaTime);

                    if (part.position == original_positions[i])
                    {
                        parts_done++;
                    }

                    i++;
                }

                // check if all parts have reached their destinations
                if (parts_done == transform.childCount)
                {
                    moving = false;
                    exploded = false;
                }
            }
        }
    }
}
