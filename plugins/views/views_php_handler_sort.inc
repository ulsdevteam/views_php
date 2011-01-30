<?php
// $Id$

/**
 * A handler to sort a view using PHP defined by the administrator.
 *
 * @ingroup views_sort_handlers
 */
class views_php_handler_sort extends views_handler_sort {

  /**
   * Implements views_handler#can_expose().
   */
  function can_expose() { return FALSE; }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_setup'] = array('default' => '');
    $options['php_sort'] = array('default' => '');
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_setup'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-setup'),
      '#title' => t('Setup code'),
      '#default_value' => $this->options['php_setup'],
      '#rows' => 5,
      '#description' => t('Code to run right before execution of the view. May be useful to define functions to be re-used in the value and/or output code.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong>',
    );
    views_php_check_input_access($form['php_setup']);
    $form['php_setup_variables'] = views_php_available_variables($form['php_setup'], $this->view, $this, array('$view', '$handler', '$static'));
    $form['php_sort'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-sort'),
      '#title' => t('Sort comparison code'),
      '#default_value' => $this->options['php_sort'],
      '#rows' => 5,
      '#description' => t('The comparison code must return an integer less than, equal to, or greater than zero if the first row should respectively appear before, stay where it was compared to, or appear after the second row.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong>',
    );
    views_php_check_input_access($form['php_sort']);
    $form['php_sort_variables'] = views_php_available_variables($form['php_sort'], $this->view, $this, array(
      '$row1' => t('Data of row.'),
      '$row2' => t('Data of row to compare against.'),
    ));
  }

  /**
   * Implements views_handler_sort#query().
   */
  function query() {
    // Inform views_php_views_pre_execute() to seize control over the query.
    $this->view->views_php = TRUE;
  }

  /**
   *
   * @see views_php_views_pre_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_setup'])) {
      $lamda_function = create_function('$view, $handler, &$static', $this->options['php_setup']);
      ob_start();
      $lamda_function($this->view, $this, &$this->php_static_variable);
      ob_end_clean();
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    if (!empty($this->options['php_sort']) && $this->view->style_plugin->build_sort()) {
      $lamda_function = create_function('$row1, $row2', $this->options['php_sort']);
      ob_start();
      usort($this->view->result, $lamda_function);
      ob_end_clean();
    }
  }
}
