/*
 * ome.xml.model.ListAnnotation
 *
 *-----------------------------------------------------------------------------
 *
 *  Copyright (C) @year@ Open Microscopy Environment
 *      Massachusetts Institute of Technology,
 *      National Institutes of Health,
 *      University of Dundee,
 *      University of Wisconsin-Madison
 *
 *
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *-----------------------------------------------------------------------------
 */

/*-----------------------------------------------------------------------------
 *
 * THIS IS AUTOMATICALLY GENERATED CODE.  DO NOT MODIFY.
 * Created by melissa via xsd-fu on 2010-06-03 11:40:12.532676
 *
 *-----------------------------------------------------------------------------
 */

package ome.xml.model;

import java.util.ArrayList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import ome.xml.model.enums.*;
import ome.xml.model.primitives.*;

public class ListAnnotation extends Annotation
{
	// Base: Annotation -- Name: ListAnnotation -- Type: ListAnnotation -- javaBase: Annotation -- javaType: Object

	// -- Constants --

	public static final String NAMESPACE = "http://www.openmicroscopy.org/Schemas/SA/2010-04";

	/** Logger for this class. */
	private static final Logger LOGGER =
		LoggerFactory.getLogger(ListAnnotation.class);

	// -- Instance variables --


	// Reference AnnotationRef
	private List<Annotation> annotationList = new ArrayList<Annotation>();

	// -- Constructors --

	/** Default constructor. */
	public ListAnnotation()
	{
		super();
	}

	/** 
	 * Constructs ListAnnotation recursively from an XML DOM tree.
	 * @param element Root of the XML DOM tree to construct a model object
	 * graph from.
	 * @param model Handler for the OME model which keeps track of instances
	 * and references seen during object population.
	 * @throws EnumerationException If there is an error instantiating an
	 * enumeration during model object creation.
	 */
	public ListAnnotation(Element element, OMEModel model)
	    throws EnumerationException
	{
		update(element, model);
	}

	// -- Custom content from ListAnnotation specific template --


	// -- OMEModelObject API methods --

	/** 
	 * Updates ListAnnotation recursively from an XML DOM tree. <b>NOTE:</b> No
	 * properties are removed, only added or updated.
	 * @param element Root of the XML DOM tree to construct a model object
	 * graph from.
	 * @param model Handler for the OME model which keeps track of instances
	 * and references seen during object population.
	 * @throws EnumerationException If there is an error instantiating an
	 * enumeration during model object creation.
	 */
	public void update(Element element, OMEModel model)
	    throws EnumerationException
	{
		super.update(element, model);
		String tagName = element.getTagName();
		if (!"ListAnnotation".equals(tagName))
		{
			LOGGER.debug("Expecting node name of ListAnnotation got {}", tagName);
		}
		// Element reference AnnotationRef
		List<Element> AnnotationRef_nodeList =
				getChildrenByTagName(element, "AnnotationRef");
		for (Element AnnotationRef_element : AnnotationRef_nodeList)
		{
			AnnotationRef annotationList_reference = new AnnotationRef();
			annotationList_reference.setID(AnnotationRef_element.getAttribute("ID"));
			model.addReference(this, annotationList_reference);
		}
	}

	// -- ListAnnotation API methods --

	public void link(Reference reference, OMEModelObject o)
	{
		if (reference instanceof AnnotationRef)
		{
			Annotation o_casted = (Annotation) o;
			o_casted.linkListAnnotation(this);
			annotationList.add(o_casted);
			return;
		}
		// TODO: Should be its own Exception
		throw new RuntimeException(
				"Unable to handle reference of type: " + reference.getClass());
	}


	// Reference which occurs more than once
	public int sizeOfLinkedAnnotationList()
	{
		return annotationList.size();
	}

	public List<Annotation> copyLinkedAnnotationList()
	{
		return new ArrayList<Annotation>(annotationList);
	}

	public Annotation getLinkedAnnotation(int index)
	{
		return annotationList.get(index);
	}

	public Annotation setLinkedAnnotation(int index, Annotation o)
	{
		return annotationList.set(index, o);
	}

	public boolean linkAnnotation(Annotation o)
	{
		o.linkListAnnotation(this);
		return annotationList.add(o);
	}

	public boolean unlinkAnnotation(Annotation o)
	{
		o.unlinkListAnnotation(this);
		return annotationList.remove(o);
	}

	public Element asXMLElement(Document document)
	{
		return asXMLElement(document, null);
	}

	protected Element asXMLElement(Document document, Element ListAnnotation_element)
	{
		// Creating XML block for ListAnnotation

		if (ListAnnotation_element == null)
		{
			ListAnnotation_element =
					document.createElementNS(NAMESPACE, "ListAnnotation");
		}

		if (annotationList != null)
		{
			// Reference property AnnotationRef which occurs more than once
			for (Annotation annotationList_value : annotationList)
			{
				AnnotationRef o = new AnnotationRef();
				o.setID(annotationList_value.getID());
				ListAnnotation_element.appendChild(o.asXMLElement(document));
			}
		}
		return super.asXMLElement(document, ListAnnotation_element);
	}
}